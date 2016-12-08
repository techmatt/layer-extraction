#include "Main.h"

void Recolorizer::init(const Bitmap &_imgInput)
{
	imgInput = _imgInput;
	
	SuperpixelGeneratorSuperpixel generator;
	const vector<SuperpixelCoord> superpixelCoords = generator.extract(imgInput, superpixels.assignments);

	superpixels.loadCoords(superpixelCoords);
	superpixels.computeNeighborhoods(imgInput);
	superpixels.computeNeighborhoodWeights(imgInput);
}

Bitmap Recolorizer::recolor(const Bitmap &imgEdits)
{
	superpixels.loadEdits(imgInput, imgEdits);

	vector<vec3f> newSuperpixelColors(superpixels.superpixels.size());
	for (auto &p : iterate(newSuperpixelColors))
	{
		p.value = superpixels.superpixels[p.index].targetColor;
	}

	return makeFinalRender(newSuperpixelColors);
}

Bitmap Recolorizer::makeFinalRender(const vector<vec3f>& newSuperpixelColors)
{
	ComponentTimer timer("Final render");

	Bitmap result = imgInput;
	for(auto &p : result)
	{
		const PixelNeighborhood &neighborhood = superpixels.pixelNeighborhoods(p.x, p.y);
		const size_t k = neighborhood.neighbors.size();

		vec3f sum = vec3f::origin;
		for (size_t neighborIndex = 0; neighborIndex < k; neighborIndex++)
		{
			sum += newSuperpixelColors[neighborhood.neighbors[neighborIndex]] * (float)neighborhood.weights[neighborIndex];
		}
		p.value = colorUtil::toVec4uc(sum);
	}

	return result;
}

#if 0
void Recolorizer::Init(const Bitmap &bmp)
{
    VisualizeSuperpixels(bmp, NULL, "superpixels");
	VisualizeNearestNeighbors(bmp);

    ComputeWeightMatrix(parameters);
}

Bitmap Recolorizer::Recolor(const Bitmap &bmp, const vector<PixelConstraint> &targetPixelColors, double lowQuartile, double highQuartile)
{
    vector<SuperpixelConstraint> constraints = MapPixelConstraintsToSuperpixelConstraints(targetPixelColors);

    const int superpixelCount = superpixelNeighbors.size();

    struct SuperpixelInfo
    {
        SuperpixelInfo() {}
        SuperpixelInfo(size_t _superpixelIndex, double _dist)
        {
            superpixelIndex = _superpixelIndex;
            dist = _dist;
        }
        size_t superpixelIndex;
        double dist;
    };
    vector<SuperpixelInfo> globalDistance(superpixelCount), localDistance(superpixelCount);
    for(int superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
    {
        globalDistance[superpixelIndex] = SuperpixelInfo(superpixelIndex, 20000000000.0);
    }

    vector<SuperpixelConstraint> superpixelConstraints;
    vector<vec3f> newSuperpixelColors(superpixelCount);
    for(int i = 0; i < superpixelCount; i++) newSuperpixelColors[i] = vec3f(superpixelColors[i].color);

    for(const SuperpixelConstraint &c : constraints)
    {
        ComputeSourceDistance(bmp, c.index);
        VisualizeSourceDistance(bmp);
        for(int superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
        {
            localDistance[superpixelIndex] = SuperpixelInfo(superpixelIndex, superpixelNeighbors[superpixelIndex].shortestDist);
            globalDistance[superpixelIndex].dist = Math::Min(globalDistance[superpixelIndex].dist, superpixelNeighbors[superpixelIndex].shortestDist);
        }

        localDistance.Sort([](const SuperpixelInfo &a, const SuperpixelInfo &b) { return a.dist < b.dist; });

        for(int i = 0; i < int(superpixelCount * lowQuartile); i++)
        {
            vec3f targetColor = c.targetColor;
            if(RGBColor(targetColor) == RGBColor::Magenta) targetColor = vec3f(superpixelColors[localDistance[i].superpixelIndex].color);
            superpixelConstraints.push_back(SuperpixelConstraint(localDistance[i].superpixelIndex, targetColor, appParams().userConstraintWeight));
            newSuperpixelColors[localDistance[i].superpixelIndex] = c.targetColor;
        }
    }

    globalDistance.Sort([](const SuperpixelInfo &a, const SuperpixelInfo &b) { return a.dist < b.dist; });
    
    for(int i = int(superpixelCount * highQuartile); i < superpixelCount; i++)
    {
        superpixelConstraints.push_back(SuperpixelConstraint(globalDistance[i].superpixelIndex, vec3f(superpixelColors[globalDistance[i].superpixelIndex].color), appParams().distantConstraintWeight));
        newSuperpixelColors[globalDistance[i].superpixelIndex] = vec3f(RGBColor::Magenta);
    }

    VisualizeSuperpixels(bmp, &newSuperpixelColors, "superpixelsConstraints");

    return Recolor(bmp, superpixelConstraints);
}

Bitmap Recolorizer::Recolor(const Bitmap &bmp, const vector<SuperpixelConstraint> &superpixelConstraints)
{
    ComponentTimer timer("Recoloring");

    const size_t n = superpixelColors.size();

    vector<double> newDiagonal = weightMatrixDiagonal;

    for(const SuperpixelConstraint &c : superpixelConstraints)
    {
        newDiagonal[c.index] += c.weight;
    }
    for(size_t i = 0; i < n; i++)
    {
        newDiagonal[i] += appParams().colorInertiaWeight;
    }
    for(size_t i = 0; i < n; i++)
    {
        weightMatrixTriplets[i] = Eigen::Triplet<double>(i, i, newDiagonal[i]);
    }

    Console::WriteLine("Loading Eigen matrix...");
    Eigen::SparseMatrix<double> M(n, n);
    M.setFromTriplets(weightMatrixTriplets.begin(), weightMatrixTriplets.end());

    //
    // Don't delete the triplets I need those!
    // Except that they are huge and you will run out of memory when cholesky factoring.
    //
    //weightMatrixTriplets.clear();
    
    Console::WriteLine("Starting cholesky solve...");
    //Eigen::SimplicialCholesky< Eigen::SparseMatrix<double> > choleskyFactorization(M);
    Eigen::SimplicialLDLT< Eigen::SparseMatrix<double> > choleskyFactorization(M);
    Console::WriteLine("Cholesky solve done");

    vector<vec3f> newSuperpixelColors(n);

    for(size_t featureIndex = 0; featureIndex < 3; featureIndex++)
    {
        ComponentTimer("Linear solve " + string(featureIndex));
        
        Eigen::VectorXd b(n);
        for(size_t i = 0; i < n; i++)
        {
            b[i] = appParams().colorInertiaWeight * superpixelColors[i].features[featureIndex];
        }
        for(const SuperpixelConstraint &c : superpixelConstraints)
        {
            b[c.index] += c.weight * c.targetColor[featureIndex];
        }

        Eigen::VectorXd x = choleskyFactorization.solve(b);
        
        for(size_t i = 0; i < n; i++)
        {
            newSuperpixelColors[i][featureIndex] = (float)x[i];
        }
    }

    VisualizeSuperpixels(bmp, &newSuperpixelColors, "superpixelsRecolored");
    return Recolor(bmp, newSuperpixelColors);
}

Bitmap Recolorizer::Recolor(const Bitmap &bmp, const vector<vec3f> &newSuperpixelColors) const
{
    ComponentTimer timer("Final recoloring");

    Bitmap result = bmp;

    for(size_t y = 0; y < bmp.getDimY(); y++)
    {
        for(size_t x = 0; x < bmp.getDimX(); x++)
        {
            const PixelNeighborhood &curPixel = pixelNeighborhoods(y, x);
            const size_t k = curPixel.indices.size();

            vec3f sum = vec3f::origin;
            for(size_t neighborIndex = 0; neighborIndex < k; neighborIndex++)
            {
                sum += vec3f(newSuperpixelColors[curPixel.indices[neighborIndex]]) * (float)curPixel.weights[neighborIndex];
            }
            result[y][x] = RGBColor(sum);
        }
    }
    return result;
}

void Recolorizer::ComputeWeightMatrix(const AppParameters &parameters)
{
    ComponentTimer timer("Computing weight matrix");
    const size_t n = superpixelNeighbors.size();
    SparseMatrix<double> W(n, n);

    for(size_t superpixelIndex = 0; superpixelIndex < n; superpixelIndex++)
    {
        W.PushDiagonalElement(superpixelIndex, 1.0);
        const SuperpixelNeighborhood &neighborhood = superpixelNeighbors[superpixelIndex];
        for(size_t neighborIndex = 0; neighborIndex < neighborhood.indices.size(); neighborIndex++)
        {
            W.PushElement(superpixelIndex, neighborhood.indices[neighborIndex], -neighborhood.embeddingWeights[neighborIndex]);
        }
    }

    Console::WriteLine("Building W-matrix");
    SparseMatrix<double> WMatrix = W.Transpose() * W;

    Console::WriteLine("Building triplets");
    weightMatrixDiagonal.Allocate(n);
    for(size_t rowIndex = 0; rowIndex < n; rowIndex++)
    {
        weightMatrixDiagonal[rowIndex] = WMatrix.GetElement(rowIndex, rowIndex);
        weightMatrixTriplets.push_back(Eigen::Triplet<double>(rowIndex, rowIndex, WMatrix.GetElement(rowIndex, rowIndex)));
    }

    for(size_t rowIndex = 0; rowIndex < n; rowIndex++)
    {
        const SparseRow<double> &row = WMatrix.Rows()[rowIndex];
        for(size_t colIndex = 0; colIndex < row.Data.size(); colIndex++)
        {
            if(rowIndex != row.Data[colIndex].Col)
            {
                weightMatrixTriplets.push_back(Eigen::Triplet<double>(rowIndex, row.Data[colIndex].Col, row.Data[colIndex].Entry));
            }
        }
    }
}

void Recolorizer::TestNeighborWeights(const Bitmap &bmp) const
{
    ComponentTimer timer("Testing neighborhood weights");

    for(size_t y = 0; y < bmp.getDimY(); y++)
    {
        for(size_t x = 0; x < bmp.getDimX(); x++)
        {
            const PixelNeighborhood &curPixel = pixelNeighborhoods(y, x);
            const size_t k = curPixel.indices.size();

            vec3f sum = vec3f::origin;
            for(size_t neighborIndex = 0; neighborIndex < k; neighborIndex++)
            {
                sum += vec3f(superpixelColors[curPixel.indices[neighborIndex]].color) * (float)curPixel.weights[neighborIndex];
            }

            float error = vec3f::Dist(vec3f(bmp[y][x]), sum);
            Console::WriteLine("e=" + string(error));
        }
    }
}

vector<double> Recolorizer::ComputeWeights(const vector<size_t> &indices, const float *pixelFeatures)
{
    //
    // Remember that for forming linear combinations, we consider only the color terms (3 dimensions) and not the spatial ones!
    //

    const size_t k = indices.size();
    DenseMatrix<double> z(k, 3);
    for(size_t neighborIndex = 0; neighborIndex < k; neighborIndex++)
    {
        const SuperpixelCoord &neighbor = superpixelColors[indices[neighborIndex]];
        for(size_t dimensionIndex = 0; dimensionIndex < 3; dimensionIndex++)
        {
            z(neighborIndex, dimensionIndex) = neighbor.features[dimensionIndex] - pixelFeatures[dimensionIndex];
        }
    }

    DenseMatrix<double> G = z * z.Transpose();

    //
    // Add weight regularization term
    //
    for(size_t neighborIndex = 0; neighborIndex < k; neighborIndex++)
    {
        G(neighborIndex, neighborIndex) += appParams().weightRegularizationTerm;
    }

    G.InvertInPlace();

    vector<double> columnVector(k, 1.0);
    vector<double> result;

    DenseMatrix<double>::Multiply(result, G, columnVector);

    result.Scale(1.0 / result.Sum());

    return result;
}

void Recolorizer::ComputeSourceDistance(const Bitmap &bmp, size_t sourceSuperpixelIndex)
{
    int n = superpixelNeighbors.size();

    for(SuperpixelNeighborhood &s : superpixelNeighbors)
    {
        s.shortestDist = 1000000000.0;
        s.visited = false;
    }

    priority_queue<SuperpixelQueueEntry> queue;
    superpixelNeighbors[sourceSuperpixelIndex].shortestDist = 0.0;
    queue.push(SuperpixelQueueEntry(superpixelNeighbors[sourceSuperpixelIndex], 0.0));

    while(!queue.empty())
    {
        SuperpixelQueueEntry curEntry = queue.top();
        queue.pop();

        if(!curEntry.n->visited)
        {
            curEntry.n->visited = true;
            for(size_t neighborIndex = 0; neighborIndex < curEntry.n->indices.size(); neighborIndex++)
            {
                SuperpixelNeighborhood &curNeighbor = superpixelNeighbors[curEntry.n->indices[neighborIndex]];
                double newDist = curEntry.n->shortestDist + curEntry.n->similarityWeights[neighborIndex];
                if(newDist < curNeighbor.shortestDist)
                {
                    curNeighbor.shortestDist = newDist;
                    queue.push(SuperpixelQueueEntry(curNeighbor, curNeighbor.shortestDist));
                }
            }
        }
    }
}

void Recolorizer::VisualizeSourceDistance(const Bitmap &bmp) const
{
    Bitmap result(bmp.getDimX(), bmp.getDimY());
    result.Clear(RGBColor::Black);

    double max = 2.0;
    //for(auto &s : superpixelNeighbors) max = Math::Max(max, s.shortestDist);

    for(size_t superpixelIndex = 0; superpixelIndex < superpixelColors.size(); superpixelIndex++)
    {
        const SuperpixelCoord &superpixelColor = superpixelColors[superpixelIndex];
        result[superpixelColor.coord.y][superpixelColor.coord.x] = RGBColor::Interpolate(RGBColor::Blue, RGBColor::Red, float(superpixelNeighbors[superpixelIndex].shortestDist / max));
    }
    result.SavePNG("../Results/sourceDistance.png");
}

void Recolorizer::VisualizeSuperpixels(const Bitmap &bmp, const vector<vec3f> *newSuperpixelColors, const string &filename) const
{
    Bitmap result(bmp.getDimX(), bmp.getDimY());
    result.Clear(RGBColor::Black);

    for(size_t superpixelIndex = 0; superpixelIndex < superpixelColors.size(); superpixelIndex++)
    {
        const SuperpixelCoord &superpixel = superpixelColors[superpixelIndex];
        if(newSuperpixelColors == NULL)
            result[superpixel.coord.y][superpixel.coord.x] = superpixel.color;
        else
            result[superpixel.coord.y][superpixel.coord.x] = RGBColor((*newSuperpixelColors)[superpixelIndex]);
    }
    result.SavePNG("../Results/" + filename + ".png");
}

void Recolorizer::VisualizeNearestNeighbors(const Bitmap &bmp) const
{
    const size_t debugPixelCount = 20;
    for(size_t debugPixelIndex = 0; debugPixelIndex < debugPixelCount; debugPixelIndex++)
    {
        int x = rand() % (bmp.getDimX() - 2) + 1;
        int y = rand() % (bmp.getDimY() - 2) + 1;

        const PixelNeighborhood &curPixel = pixelNeighborhoods(y, x);

        Bitmap result(bmp.getDimX(), bmp.getDimY());
        result.Clear(RGBColor::Black);

        result[y + 0][x + 0] = bmp[y][x];
        result[y + 1][x + 0] = bmp[y][x];
        result[y - 1][x + 0] = bmp[y][x];
        result[y + 0][x + 1] = bmp[y][x];
        result[y + 0][x - 1] = bmp[y][x];

        for(size_t neighborIndex : curPixel.indices)
        {
            const SuperpixelCoord &superpixel = superpixelColors[neighborIndex];
            result[superpixel.coord.y][superpixel.coord.x] = superpixel.color;
        }

        result.SavePNG("../Results/pixel" + string(debugPixelIndex) + ".png");
    }
}
#endif