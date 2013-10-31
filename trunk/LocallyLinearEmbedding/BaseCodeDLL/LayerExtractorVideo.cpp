#include "Main.h"

void LayerExtractorVideo::Init(const AppParameters &parameters, const Video &video)
{
    ComputeSuperpixels(parameters, video);
    VisualizeSuperpixels(parameters, video, 0, NULL, "superpixels");

    ComputeNearestNeighbors(parameters, video);
    VisualizeNearestNeighbors(parameters, video, 0);

    ComputeNeighborWeights(parameters, video);

    ComputeWeightMatrix(parameters);

    pass = 0;
}

void LayerExtractorVideo::AddNegativeConstraints(const AppParameters &parameters, const Video &video, LayerSet &result)
{
	String key = "negative";
	if (!result.constraints.ContainsKey(key))
		result.constraints.Add(key, Vector<SuperpixelLayerConstraint>());

    map<UINT64,UINT> activeConstraints;
    for(UINT constraintIndex = 0; constraintIndex < result.constraints[key].Length(); constraintIndex++)
    {
        const auto &c = result.constraints[key][constraintIndex];
        activeConstraints[UINT64(c.index) + (UINT64(c.layerIndex) << 32)] = constraintIndex;
    }

    const UINT layerCount = result.layers.Length();
    const UINT superpixelCount = superpixelColors.Length();
    for(UINT layerIndex = 0; layerIndex < layerCount; layerIndex++)
    {
        Layer &curLayer = result.layers[layerIndex];
        for(UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
        {
            UINT64 constraintHash = UINT64(superpixelIndex) + (UINT64(layerIndex) << 32);
            if(curLayer.superpixelWeights[superpixelIndex] < 0.0)
            {
                if(activeConstraints.count(constraintHash) == 0)
                {
                    result.constraints[key].PushEnd(SuperpixelLayerConstraint(superpixelIndex, layerIndex, 0.0, parameters.negativeSupressionWeight));
                }
                else
                {
                    auto &c = result.constraints[key][activeConstraints[constraintHash]];
                    if(c.target == 0.0) c.weight += parameters.negativeSupressionWeight;
                }
            }
        }
    }
    pass++;
}

void LayerExtractorVideo::InitLayersFromPalette(const AppParameters &parameters, const Video &video, const Vector<Vec3f> &palette, LayerSet &result)
{
    ComponentTimer timer("Making layers from palette");
	String key = "initial";
	if (!result.constraints.ContainsKey(key))
		result.constraints.Add(key, Vector<SuperpixelLayerConstraint>());
    
    const UINT layerCount = palette.Length();
    const UINT superpixelCount = superpixelColors.Length();
    result.layers.Allocate(layerCount);

    for(UINT layerIndex = 0; layerIndex < layerCount; layerIndex++)
    {
        Layer &curLayer = result.layers[layerIndex];
        curLayer.color = palette[layerIndex];
    }

    VisualizeLayerConstraints(parameters, video, 0, result);
}

void LayerExtractorVideo::ExtractLayers(const AppParameters &parameters, const Video &video, LayerSet &layers)
{
    ComponentTimer timer("Extracting layers");

    //
    // There are five types of constraints going on:
    //
    // *** Linear embedding constraints
    // Mi = (I - W)^T * (I - W) -> These are the constraints that values should be a linear combination of their neighborhood. There is one of these for each set of layer weights
    // This forms a matrix which looks like (for 3 layers):
    // (M0 0  0 )(w0) = (0)
    // (0  M1 0 )(w1) = (0)
    // (0  0  M2)(w2) = (0)
    // Call this super-matrix Q
    //
    // *** Explicit layer constraints
    // Some superpixels in each layer are constrained directly to have specific weights.
    // lhs: C = lp x lp (diagonal matrix)
    // rhs: C * target
	// There may be multiple C matrices for different sets of constraints
    //
    // *** Sum-to-one constraints
    // The sum of weights for each superpixel should be 1.
    // lhs: S = p x lp
    // rhs: S^T * s
    //
    // *** Layer reconstruction constraints
    // The linear combination of colors should reconstruct the superpixel colors.
    // lhs: R = 3p x lp
    // rhs: R^T * r
    //
    // *** Regularization constraints
    // All else being equal, weights should be as small as possible to encourage near-zero regions.
    //
    // *** Least squares solutions
    // Ax = b
    // A^TAx = A^Tb
    //
    // *** Final matrix
    // (Q + sum(wC C) + wS S^T S + wR R^T R) x = sum(wC Crhs) + wS S^T Srhs + wR R^T Rrhs
	// (WBase + wC C)x = wC Crhs
    // where wC, wS, wR are three weights
    //

    //Vector<Eigen::Triplet<double> > weightMatrixTriplets;
    const UINT superpixelCount = superpixelColors.Length();
    const UINT layerCount = layers.layers.Length();

    //
    // Build the linear embedding constraint matrix Q
    //
    SparseMatrix<double> Q(layerCount * superpixelCount);
    for(UINT layerIndex = 0; layerIndex < layerCount; layerIndex++)
    {
        const UINT layerOffset = layerIndex * superpixelCount;
        for(UINT row = 0; row < superpixelCount; row++)
        {
            const SparseRow<double> &curRow = WBase[row];
            for(const SparseElement<double> &e : curRow.Data)
            {
                Q.PushElement(row + layerOffset, e.Col + layerOffset, parameters.manifoldWeight*e.Entry);
            }
        }
    }

    //
    // Add simple (diagonal) constraints to lhs and lambda * target to rhs
    //
    Vector<double> Crhs(layerCount * superpixelCount, 0.0);
	Vector<String> constraintKeys = layers.constraints.Keys();
	for (String key : constraintKeys)
	{
		for(UINT constraintIndex = 0; constraintIndex < layers.constraints[key].Length(); constraintIndex++)
		{
			const SuperpixelLayerConstraint &constraint = layers.constraints[key][constraintIndex];
			int row = constraint.index + constraint.layerIndex * superpixelCount;
        
			Q.PushDiagonalElement(row, constraint.weight);
			Crhs[row] += constraint.weight * constraint.target;
		}
	}
    for(UINT row = 0; row < layerCount * superpixelCount; row++)
    {
        Q.PushDiagonalElement(row, parameters.regularizationWeight);
    }

    //
    // Construct the sum-to-one matrix
    //
    SparseMatrix<double> S(superpixelCount, layerCount * superpixelCount);
    Vector<double> Srhs(superpixelCount, parameters.sumToOneWeight);
    for(UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
    {
        for(UINT layerIndex = 0; layerIndex < layerCount; layerIndex++)
        {
            S.PushElement(superpixelIndex, superpixelIndex + layerIndex * superpixelCount, 1.0);
        }
    }
    SparseMatrix<double> STranspose = S.Transpose();
    SparseMatrix<double> STS = STranspose * S;
    STS *= parameters.sumToOneWeight;
    Q += STS;

    //
    // Construct the reconstruction matrix
    //
    SparseMatrix<double> R(3 * superpixelCount, layerCount * superpixelCount);
    Vector<double> Rrhs(3 * superpixelCount, 0.0);
    for(UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
    {
        Vec3f superpixelColor = Vec3f(superpixelColors[superpixelIndex].color);

        for(UINT channelIndex = 0; channelIndex < 3; channelIndex++)
        {
            UINT constraintRow = superpixelIndex * 3 + channelIndex;
            Rrhs[constraintRow] = parameters.reconstructionWeight * superpixelColor[channelIndex];
            for(UINT layerIndex = 0; layerIndex < layerCount; layerIndex++)
            {
                R.PushElement(constraintRow, superpixelIndex + layerIndex * superpixelCount, layers.layers[layerIndex].color[channelIndex]);
            }
        }
    }
    SparseMatrix<double> RTranspose = R.Transpose();
    SparseMatrix<double> RTR = RTranspose * R;
    RTR *= parameters.reconstructionWeight;
    Q += RTR;

    Vector<double> b = Crhs + STranspose * Srhs + RTranspose * Rrhs;

    EigenSolver solver;
    Vector<double> x = solver.Solve(Q, b, EigenSolver::ConjugateGradient_Diag);

    for(UINT layerIndex = 0; layerIndex < layerCount; layerIndex++)
    {
        Layer &curLayer = layers.layers[layerIndex];
        curLayer.superpixelWeights.Allocate(superpixelCount);
        for(UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
        {
            curLayer.superpixelWeights[superpixelIndex] = x[superpixelIndex + layerIndex * superpixelCount];
        }
    }

    if(Constants::dumpLayerImages)
    {
        VisualizeLayers(parameters, video, 0, layers);
        VisualizeReconstruction(parameters, video, 0, layers);
    }
}

Bitmap LayerExtractorVideo::RecolorSuperpixels(const Video &video, UINT frameIndex, const Vector<Vec3f> &newSuperpixelColors) const
{
    Bitmap result = video.frames[frameIndex];

    for(UINT y = 0; y < result.Height(); y++)
    {
        for(UINT x = 0; x < result.Width(); x++)
        {
            const PixelNeighborhood &curPixel = pixelNeighborhoods[frameIndex](y, x);
            const UINT k = curPixel.indices.Length();

            Vec3f sum = Vec3f::Origin;
            for(UINT neighborIndex = 0; neighborIndex < k; neighborIndex++)
            {
                sum += Vec3f(newSuperpixelColors[curPixel.indices[neighborIndex]]) * (float)curPixel.weights[neighborIndex];
            }
            result[y][x] = RGBColor(sum);
        }
    }
    return result;
}

Bitmap LayerExtractorVideo::RecolorLayers(const Video &video, UINT frameIndex, const LayerSet &layers, const Vector<RGBColor> &newLayerColors) const
{
    const UINT superpixelCount = superpixelColors.Length();
    Vector<Vec3f> newColors(superpixelCount, Vec3f::Origin);
    for(UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
    {
        for(UINT layerIndex = 0; layerIndex < layers.layers.Length(); layerIndex++)
        {
            newColors[superpixelIndex] += (float)layers.layers[layerIndex].superpixelWeights[superpixelIndex] * Vec3f(newLayerColors[layerIndex]);
        }
    }

    return RecolorSuperpixels(video, frameIndex, newColors);
}

void LayerExtractorVideo::ComputeSuperpixels(const AppParameters &parameters, const Video &video)
{
    ComponentTimer timer("Computing superpixels");

    //SuperpixelExtractorPeriodic extractor;
    SuperpixelExtractorVideoPeriodic extractor;
    superpixelColors = extractor.Extract(parameters, video);
}

void LayerExtractorVideo::ComputeWeightMatrix(const AppParameters &parameters)
{
    ComponentTimer timer("Computing weight matrix");
    const UINT n = superpixelNeighbors.Length();
    SparseMatrix<double> W(n, n);

    for(UINT superpixelIndex = 0; superpixelIndex < n; superpixelIndex++)
    {
        W.PushDiagonalElement(superpixelIndex, 1.0);
        const SuperpixelNeighborhood &neighborhood = superpixelNeighbors[superpixelIndex];
        for(UINT neighborIndex = 0; neighborIndex < neighborhood.indices.Length(); neighborIndex++)
        {
            W.PushElement(superpixelIndex, neighborhood.indices[neighborIndex], -neighborhood.embeddingWeights[neighborIndex]);
        }
    }

    Console::WriteLine("Building W-matrix");
    WBase = W.Transpose() * W;
}

void LayerExtractorVideo::ComputeNearestNeighbors(const AppParameters &parameters, const Video &video)
{
    ComponentTimer timer("Computing nearest neighbors");

    KDTree tree;

    Vector<const float*> points;
    for(const ColorCoordinateVideo &superpixel : superpixelColors)
    {
        points.PushEnd(superpixel.features);
    }

    tree.BuildTree(points, 5, Math::Max(parameters.pixelNeighborCount, parameters.superpixelNeighborCount) + 1);

    pixelNeighborhoods.Allocate(video.frames.Length());
    for(Grid<PixelNeighborhood> &n : pixelNeighborhoods) n.Allocate(video.Height(), video.Width());

    for(UINT frameIndex = 0; frameIndex < video.frames.Length(); frameIndex++)
    {
        for(UINT y = 0; y < video.Height(); y++)
        {
            for(UINT x = 0; x < video.Width(); x++)
            {
                ColorCoordinateVideo curCoord(parameters, video.frames[frameIndex][y][x], Vec2i(x, y), frameIndex, video.Width(), video.Height());
                tree.KNearest(curCoord.features, parameters.pixelNeighborCount, pixelNeighborhoods[frameIndex](y, x).indices, 0.0f);
            }
        }
    }

    superpixelNeighbors.Allocate(superpixelColors.Length());

    for(UINT superpixelIndex = 0; superpixelIndex < superpixelColors.Length(); superpixelIndex++)
    {
        SuperpixelNeighborhood &b = superpixelNeighbors[superpixelIndex];
        tree.KNearest(superpixelColors[superpixelIndex].features, parameters.superpixelNeighborCount + 1, b.indices, 0.0f);
        if(b.indices.Contains(superpixelIndex))
        {
            b.indices.RemoveSwap(b.indices.FindFirstIndex(superpixelIndex));
        }
        else
        {
            b.indices.PopEnd();
        }
    }
}

void LayerExtractorVideo::TestNeighborWeights(const AppParameters &parameters, const Video &video) const
{
    ComponentTimer timer("Testing neighborhood weights");

    for(UINT frameIndex = 0; frameIndex < video.frames.Length(); frameIndex++)
    {
        for(UINT y = 0; y < video.Height(); y++)
        {
            for(UINT x = 0; x < video.Width(); x++)
            {
                const PixelNeighborhood &curPixel = pixelNeighborhoods[frameIndex](y, x);
                const UINT k = curPixel.indices.Length();

                Vec3f sum = Vec3f::Origin;
                for(UINT neighborIndex = 0; neighborIndex < k; neighborIndex++)
                {
                    sum += Vec3f(superpixelColors[curPixel.indices[neighborIndex]].color) * (float)curPixel.weights[neighborIndex];
                }

                float error = Vec3f::Dist(Vec3f(video.frames[frameIndex][y][x]), sum);
                Console::WriteLine("e=" + String(error));
            }
        }
    }
}

Vector<double> LayerExtractorVideo::ComputeWeights(const AppParameters &parameters, const Vector<UINT> &indices, const float *pixelFeatures)
{
    //
    // Remember that for forming linear combinations, we consider only the color terms (3 dimensions) and not the spatial ones!
    //

    const UINT k = indices.Length();
    DenseMatrix<double> z(k, 3);
    for(UINT neighborIndex = 0; neighborIndex < k; neighborIndex++)
    {
        const ColorCoordinateVideo &neighbor = superpixelColors[indices[neighborIndex]];
        for(UINT dimensionIndex = 0; dimensionIndex < 3; dimensionIndex++)
        {
            z(neighborIndex, dimensionIndex) = neighbor.features[dimensionIndex] - pixelFeatures[dimensionIndex];
        }
    }

    DenseMatrix<double> G = z * z.Transpose();

    //
    // Add weight regularization term
    //
    for(UINT neighborIndex = 0; neighborIndex < k; neighborIndex++)
    {
        G(neighborIndex, neighborIndex) += parameters.weightRegularizationTerm;
    }

    G.InvertInPlace();

    Vector<double> columnVector(k, 1.0);
    Vector<double> result;

    DenseMatrix<double>::Multiply(result, G, columnVector);

    result.Scale(1.0 / result.Sum());

    return result;
}

void LayerExtractorVideo::ComputeNeighborWeights(const AppParameters &parameters, const Video &video)
{
    ComponentTimer timer("Computing neighborhood weights");

    Console::WriteLine("Computing pixel weights");
    for(UINT frameIndex = 0; frameIndex < video.frames.Length(); frameIndex++)
    {
        for(UINT y = 0; y < video.Height(); y++)
        {
            for(UINT x = 0; x < video.Width(); x++)
            {
                PixelNeighborhood &curPixel = pixelNeighborhoods[frameIndex](y, x);
                const UINT k = curPixel.indices.Length();

                ColorCoordinateVideo curPixelCoordinate(parameters, video.frames[frameIndex][y][x], Vec2i(x, y), frameIndex, video.Width(), video.Height());
                curPixel.weights = ComputeWeights(parameters, curPixel.indices, curPixelCoordinate.features);
            }
        }
    }

    Console::WriteLine("Computing superpixel weights");
    for(UINT superpixelIndex = 0; superpixelIndex < superpixelColors.Length(); superpixelIndex++)
    {
        SuperpixelNeighborhood &n = superpixelNeighbors[superpixelIndex];
        
        n.embeddingWeights = ComputeWeights(parameters, superpixelNeighbors[superpixelIndex].indices, superpixelColors[superpixelIndex].features);

        n.similarityWeights.Allocate(n.indices.Length());
        for(UINT neighborIndex = 0; neighborIndex < n.similarityWeights.Length(); neighborIndex++)
        {
            n.similarityWeights[neighborIndex] = Vec3f::Dist(Vec3f(superpixelColors[superpixelIndex].color), Vec3f(superpixelColors[n.indices[neighborIndex]].color));
        }
    }
}

void LayerExtractorVideo::VisualizeSuperpixels(const AppParameters &parameters, const Video &video, UINT frameIndex, const Vector<Vec3f> *newSuperpixelColors, const String &filename) const
{
    /*Bitmap result(bmp.Width(), bmp.Height());
    result.Clear(RGBColor::Black);

    for(UINT superpixelIndex = 0; superpixelIndex < superpixelColors.Length(); superpixelIndex++)
    {
        const ColorCoordinate &superpixel = superpixelColors[superpixelIndex];
        if(newSuperpixelColors == NULL)
            result[superpixel.coord.y][superpixel.coord.x] = superpixel.color;
        else
            result[superpixel.coord.y][superpixel.coord.x] = RGBColor((*newSuperpixelColors)[superpixelIndex]);
    }
    result.SavePNG("../Results/" + filename + ".png");*/
}

void LayerExtractorVideo::VisualizeNearestNeighbors(const AppParameters &parameters, const Video &video, UINT frameIndex) const
{
    /*const UINT debugPixelCount = 5;
    for(UINT debugPixelIndex = 0; debugPixelIndex < debugPixelCount; debugPixelIndex++)
    {
        int x = rand() % (bmp.Width() - 2) + 1;
        int y = rand() % (bmp.Height() - 2) + 1;

        const PixelNeighborhood &curPixel = pixelNeighborhoods(y, x);

        Bitmap result(bmp.Width(), bmp.Height());
        result.Clear(RGBColor::Black);
		
		for(UINT y = 0; y < bmp.Height(); y++) for(UINT x = 0; x < bmp.Width(); x++) result[y][x] = RGBColor(Vec3f(bmp[y][x]) * 0.2f);

        result[y + 0][x + 0] = bmp[y][x];
        result[y + 1][x + 0] = bmp[y][x];
        result[y - 1][x + 0] = bmp[y][x];
        result[y + 0][x + 1] = bmp[y][x];
        result[y + 0][x - 1] = bmp[y][x];

        for(UINT neighborIndex : curPixel.indices)
        {
            const ColorCoordinate &superpixel = superpixelColors[neighborIndex];
            result[superpixel.coord.y][superpixel.coord.x] = superpixel.color;
        }

        result.SavePNG("../Results/pixel" + String(debugPixelIndex) + ".png");
    }

	for (UINT debugSuperpixelIndex=0; debugSuperpixelIndex < debugPixelCount; debugSuperpixelIndex++)
	{
		int randIdx = rand() % superpixelColors.Length();
		const SuperpixelNeighborhood &curSPixel = superpixelNeighbors[randIdx];
		
		Bitmap result(bmp.Width(), bmp.Height());
		result.Clear(RGBColor::Black);
		
		for(UINT y = 0; y < bmp.Height(); y++) for(UINT x = 0; x < bmp.Width(); x++) result[y][x] = RGBColor(Vec3f(bmp[y][x]) * 0.2f);

		int x = superpixelColors[randIdx].coord.x;
		int y = superpixelColors[randIdx].coord.y;

		result[y + 0][x + 0] = bmp[y][x];
        result[y + 1][x + 0] = bmp[y][x];
        result[y - 1][x + 0] = bmp[y][x];
        result[y + 0][x + 1] = bmp[y][x];
        result[y + 0][x - 1] = bmp[y][x];
	
		for (UINT neighborIndex : curSPixel.indices)
		{
			const ColorCoordinate &superpixel = superpixelColors[neighborIndex];
			result[superpixel.coord.y][superpixel.coord.x] = superpixel.color;
		}
		result.SavePNG("../Results/superpixel"+String(debugSuperpixelIndex)+".png");
	
	}*/
}

void LayerExtractorVideo::VisualizeLayerConstraints(const AppParameters &parameters, const Video &video, UINT frameIndex, const LayerSet &layers) const
{
	/*String key = "initial";

    const UINT paletteHeight = 40;
    Bitmap result(bmp.Width(), bmp.Height() + paletteHeight);
    
    result.Clear(RGBColor::Black);

    for(UINT y = 0; y < bmp.Height(); y++) for(UINT x = 0; x < bmp.Width(); x++) result[y][x] = RGBColor(Vec3f(bmp[y][x]) * 0.2f);
    
    AliasRender render;
	const Vector<SuperpixelLayerConstraint>& constraints = layers.constraints[key];
    for(const auto &constraint : constraints)
    {
        if(constraint.target > 0.0)
        {
            RGBColor color = RGBColor(layers.layers[constraint.layerIndex].color);
            RGBColor borderColor = color;
            if(Vec3f(color).Length() < 0.3) borderColor = RGBColor::White;
        
            render.DrawRect(result, Rectangle2i::ConstructFromCenterVariance(superpixelColors[constraint.index].coord, Vec2i(2, 2)), color, borderColor);
        }
    }

    int layerWidth = Math::Ceiling((double)bmp.Width() / (double)layers.layers.Length());
    for(UINT layerIndex = 0; layerIndex < layers.layers.Length(); layerIndex++)
    {
        for(int x = layerIndex * layerWidth; x < (int)(layerIndex + 1) * layerWidth; x++)
        {
            for(int y = (int)bmp.Height(); y < (int)result.Height(); y++)
            {
                if(result.ValidCoordinates(x, y)) result[y][x] = RGBColor(layers.layers[layerIndex].color);
            }
        }
    }

    result.SavePNG("../Results/LayerConstraints.png");*/
}

void LayerExtractorVideo::VisualizeReconstruction(const AppParameters &parameters, const Video &video, UINT frameIndex, const LayerSet &layers) const
{
    /*const UINT superpixelCount = superpixelColors.Length();
    Vector<Vec3f> newColors(superpixelCount, Vec3f::Origin);
    for(UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
    {
        for(const Layer &l : layers.layers)
        {
            newColors[superpixelIndex] += (float)l.superpixelWeights[superpixelIndex] * l.color;
        }
    }

    RecolorSuperpixels(bmp, newColors).SavePNG("../Results/Reconstruction_P" + String(pass) + ".png");*/
}

void LayerExtractorVideo::VisualizeLayers(const AppParameters &parameters, const Video &video, UINT frameIndex, const LayerSet &layers) const
{
  //  AliasRender render;
  //  for(UINT layerIndex = 0; layerIndex < layers.layers.Length(); layerIndex++)
  //  {
  //      const Layer &curLayer = layers.layers[layerIndex];
  //      
  //      const UINT superpixelCount = superpixelColors.Length();
  //      Vector<Vec3f> newColors(superpixelCount);
  //      for(UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
  //      {
  //          newColors[superpixelIndex] = Vec3f(RGBColor::Interpolate(RGBColor::Black, RGBColor::White, (float)curLayer.superpixelWeights[superpixelIndex]));
  //      }
  //      
  //      Bitmap smoothBmp = RecolorSuperpixels(bmp, newColors);
  //      Bitmap discreteBmp = smoothBmp;
		//discreteBmp.Clear(RGBColor::Black);

  //      for(UINT superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
  //      {
  //          float weight = (float)curLayer.superpixelWeights[superpixelIndex];
  //          Vec3f c = Vec3f(RGBColor::Interpolate(RGBColor::Black, RGBColor::White, weight));
  //          
  //          if(weight > 1.02) c = Vec3f(RGBColor::Red);
  //          if(weight < -0.02) c = Vec3f(RGBColor::Blue);
  //          render.DrawRect(discreteBmp, Rectangle2i::ConstructFromCenterVariance(superpixelColors[superpixelIndex].coord, Vec2i(2, 2)), RGBColor(c), RGBColor(c));//RGBColor(0, 128, 0));
  //      }
  //      
  //      for(UINT y = 0; y < smoothBmp.Height(); y++) for(UINT x = 0; x < 10; x++)
  //      {
  //          smoothBmp[y][x] = RGBColor(curLayer.color);
  //          discreteBmp[y][x] = RGBColor(curLayer.color);
  //      }
  //      discreteBmp.SavePNG("../Results/LayerD" + String(layerIndex) + "_P" + String(pass) + ".png");
  //      smoothBmp.SavePNG("../Results/LayerS" + String(layerIndex) + "_P" + String(pass) + ".png");
  //  }
}
