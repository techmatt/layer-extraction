#include "Main.h"

void Recolorizer::Init(const AppParameters &parameters, const Bitmap &bmp)
{
    ComputeSuperpixel(parameters, bmp);
    VisualizeSuperpixels(parameters, bmp, NULL, "superpixels");

    ComputeNearestNeighbors(parameters, bmp);
    VisualizeNearestNeighbors(parameters, bmp);

    ComputeNeighborWeights(parameters, bmp);
    //TestNeighborWeights(parameters, bmp);

    ComputeWeightMatrix(parameters);
}

Bitmap Recolorizer::Recolor(const AppParameters &parameters, const Bitmap &bmp, const Vector<PixelConstraint> &targetPixelColors, double lowQuartile, double highQuartile)
{
    Vector<SuperpixelConstraint> constraints = MapPixelConstraintsToSuperpixelConstraints(parameters, targetPixelColors);

    const int superpixelCount = superpixelNeighbors.Length();

    struct SuperpixelInfo
    {
        SuperpixelInfo() {}
        SuperpixelInfo(UINT _superpixelIndex, double _dist)
        {
            superpixelIndex = _superpixelIndex;
            dist = _dist;
        }
        UINT superpixelIndex;
        double dist;
    };
    Vector<SuperpixelInfo> globalDistance(superpixelCount), localDistance(superpixelCount);
    for(int superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
    {
        globalDistance[superpixelIndex] = SuperpixelInfo(superpixelIndex, 20000000000.0);
    }

    Vector<SuperpixelConstraint> superpixelConstraints;
    Vector<Vec3f> newSuperpixelColors(superpixelCount);
    for(int i = 0; i < superpixelCount; i++) newSuperpixelColors[i] = Vec3f(superpixelColors[i].color);

    for(const SuperpixelConstraint &c : constraints)
    {
        ComputeSourceDistance(parameters, bmp, c.index);
        VisualizeSourceDistance(parameters, bmp);
        for(int superpixelIndex = 0; superpixelIndex < superpixelCount; superpixelIndex++)
        {
            localDistance[superpixelIndex] = SuperpixelInfo(superpixelIndex, superpixelNeighbors[superpixelIndex].shortestDist);
            globalDistance[superpixelIndex].dist = Math::Min(globalDistance[superpixelIndex].dist, superpixelNeighbors[superpixelIndex].shortestDist);
        }

        localDistance.Sort([](const SuperpixelInfo &a, const SuperpixelInfo &b) { return a.dist < b.dist; });

        for(int i = 0; i < int(superpixelCount * lowQuartile); i++)
        {
            Vec3f targetColor = c.targetColor;
            if(RGBColor(targetColor) == RGBColor::Magenta) targetColor = Vec3f(superpixelColors[localDistance[i].superpixelIndex].color);
            superpixelConstraints.PushEnd(SuperpixelConstraint(localDistance[i].superpixelIndex, targetColor, parameters.userConstraintWeight));
            newSuperpixelColors[localDistance[i].superpixelIndex] = c.targetColor;
        }
    }

    globalDistance.Sort([](const SuperpixelInfo &a, const SuperpixelInfo &b) { return a.dist < b.dist; });
    
    for(int i = int(superpixelCount * highQuartile); i < superpixelCount; i++)
    {
        superpixelConstraints.PushEnd(SuperpixelConstraint(globalDistance[i].superpixelIndex, Vec3f(superpixelColors[globalDistance[i].superpixelIndex].color), parameters.distantConstraintWeight));
        newSuperpixelColors[globalDistance[i].superpixelIndex] = Vec3f(RGBColor::Magenta);
    }

    VisualizeSuperpixels(parameters, bmp, &newSuperpixelColors, "superpixelsConstraints");

    return Recolor(parameters, bmp, superpixelConstraints);
}

Bitmap Recolorizer::Recolor(const AppParameters &parameters, const Bitmap &bmp, const Vector<PixelConstraint> &targetPixelColors)
{
    return Recolor(parameters, bmp, MapPixelConstraintsToSuperpixelConstraints(parameters, targetPixelColors));
}

Bitmap Recolorizer::Recolor(const AppParameters &parameters, const Bitmap &bmp, const Vector<SuperpixelConstraint> &superpixelConstraints)
{
    ComponentTimer timer("Recoloring");

    const UINT n = superpixelColors.Length();

    Vector<double> newDiagonal = weightMatrixDiagonal;

    for(const SuperpixelConstraint &c : superpixelConstraints)
    {
        newDiagonal[c.index] += c.weight;
    }
    for(UINT i = 0; i < n; i++)
    {
        newDiagonal[i] += parameters.colorInertiaWeight;
    }
    for(UINT i = 0; i < n; i++)
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
    //weightMatrixTriplets.FreeMemory();
    
    Console::WriteLine("Starting cholesky solve...");
    //Eigen::SimplicialCholesky< Eigen::SparseMatrix<double> > choleskyFactorization(M);
    Eigen::SimplicialLDLT< Eigen::SparseMatrix<double> > choleskyFactorization(M);
    Console::WriteLine("Cholesky solve done");

    Vector<Vec3f> newSuperpixelColors(n);

    for(UINT featureIndex = 0; featureIndex < 3; featureIndex++)
    {
        ComponentTimer("Linear solve " + String(featureIndex));
        
        Eigen::VectorXd b(n);
        for(UINT i = 0; i < n; i++)
        {
            b[i] = parameters.colorInertiaWeight * superpixelColors[i].features[featureIndex];
        }
        for(const SuperpixelConstraint &c : superpixelConstraints)
        {
            b[c.index] += c.weight * c.targetColor[featureIndex];
        }

        Eigen::VectorXd x = choleskyFactorization.solve(b);
        
        for(UINT i = 0; i < n; i++)
        {
            newSuperpixelColors[i][featureIndex] = (float)x[i];
        }
    }

    VisualizeSuperpixels(parameters, bmp, &newSuperpixelColors, "superpixelsRecolored");
    return Recolor(bmp, newSuperpixelColors);
}

Bitmap Recolorizer::Recolor(const Bitmap &bmp, const Vector<Vec3f> &newSuperpixelColors) const
{
    ComponentTimer timer("Final recoloring");

    Bitmap result = bmp;

    for(UINT y = 0; y < bmp.Height(); y++)
    {
        for(UINT x = 0; x < bmp.Width(); x++)
        {
            const PixelNeighborhood &curPixel = pixelNeighborhoods(y, x);
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

Vector<SuperpixelConstraint> Recolorizer::MapPixelConstraintsToSuperpixelConstraints(const AppParameters &parameters, const Vector<PixelConstraint> &targetPixelColors) const
{
    map<UINT, SuperpixelConstraint> superpixelConstraints;
    for(const PixelConstraint &p : targetPixelColors)
    {
        UINT nearestSuperpixel = pixelNeighborhoods(p.coord.y, p.coord.x).indices[0];
        if(superpixelConstraints.find(nearestSuperpixel) == superpixelConstraints.end())
        {
            superpixelConstraints[nearestSuperpixel] = SuperpixelConstraint(nearestSuperpixel);
        }
        superpixelConstraints[nearestSuperpixel].count++;
        superpixelConstraints[nearestSuperpixel].targetColor += p.targetColor;
    }

    Vector<SuperpixelConstraint> result;
    for(auto &entry : superpixelConstraints)
    {
        SuperpixelConstraint curConstraint = entry.second;
        curConstraint.targetColor /= float(curConstraint.count);
        result.PushEnd(curConstraint);
    }
    return result;
}

void Recolorizer::ComputeSuperpixel(const AppParameters &parameters, const Bitmap &bmp)
{
    ComponentTimer timer("Computing superpixels");

    //SuperpixelExtractorPeriodic extractor;
    SuperpixelExtractorSuperpixel extractor;
    superpixelColors = extractor.Extract(parameters, bmp);
}

void Recolorizer::ComputeWeightMatrix(const AppParameters &parameters)
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
    SparseMatrix<double> WMatrix = W.Transpose() * W;

    Console::WriteLine("Building triplets");
    weightMatrixDiagonal.Allocate(n);
    for(UINT rowIndex = 0; rowIndex < n; rowIndex++)
    {
        weightMatrixDiagonal[rowIndex] = WMatrix.GetElement(rowIndex, rowIndex);
        weightMatrixTriplets.PushEnd(Eigen::Triplet<double>(rowIndex, rowIndex, WMatrix.GetElement(rowIndex, rowIndex)));
    }

    for(UINT rowIndex = 0; rowIndex < n; rowIndex++)
    {
        const SparseRow<double> &row = WMatrix.Rows()[rowIndex];
        for(UINT colIndex = 0; colIndex < row.Data.Length(); colIndex++)
        {
            if(rowIndex != row.Data[colIndex].Col)
            {
                weightMatrixTriplets.PushEnd(Eigen::Triplet<double>(rowIndex, row.Data[colIndex].Col, row.Data[colIndex].Entry));
            }
        }
    }
}

void Recolorizer::ComputeNearestNeighbors(const AppParameters &parameters, const Bitmap &bmp)
{
    ComponentTimer timer("Computing nearest neighbors");

    KDTree tree;

    Vector<const float*> points;
    for(const ColorCoordinate &superpixel : superpixelColors)
    {
        points.PushEnd(superpixel.features);
    }

    tree.BuildTree(points, 5, Math::Max(parameters.pixelNeighborCount, parameters.superpixelNeighborCount) + 1);

    pixelNeighborhoods.Allocate(bmp.Height(), bmp.Width());

    for(UINT y = 0; y < bmp.Height(); y++)
    {
        for(UINT x = 0; x < bmp.Width(); x++)
        {
            ColorCoordinate curCoord(parameters, bmp[y][x], Vec2i(x, y), bmp.Width(), bmp.Height());
            tree.KNearest(curCoord.features, parameters.pixelNeighborCount, pixelNeighborhoods(y, x).indices, 0.0f);
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

void Recolorizer::TestNeighborWeights(const AppParameters &parameters, const Bitmap &bmp) const
{
    ComponentTimer timer("Testing neighborhood weights");

    for(UINT y = 0; y < bmp.Height(); y++)
    {
        for(UINT x = 0; x < bmp.Width(); x++)
        {
            const PixelNeighborhood &curPixel = pixelNeighborhoods(y, x);
            const UINT k = curPixel.indices.Length();

            Vec3f sum = Vec3f::Origin;
            for(UINT neighborIndex = 0; neighborIndex < k; neighborIndex++)
            {
                sum += Vec3f(superpixelColors[curPixel.indices[neighborIndex]].color) * (float)curPixel.weights[neighborIndex];
            }

            float error = Vec3f::Dist(Vec3f(bmp[y][x]), sum);
            Console::WriteLine("e=" + String(error));
        }
    }
}

Vector<double> Recolorizer::ComputeWeights(const AppParameters &parameters, const Vector<UINT> &indices, const float *pixelFeatures)
{
    //
    // Remember that for forming linear combinations, we consider only the color terms (3 dimensions) and not the spatial ones!
    //

    const UINT k = indices.Length();
    DenseMatrix<double> z(k, 3);
    for(UINT neighborIndex = 0; neighborIndex < k; neighborIndex++)
    {
        const ColorCoordinate &neighbor = superpixelColors[indices[neighborIndex]];
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

void Recolorizer::ComputeSourceDistance(const AppParameters &parameters, const Bitmap &bmp, UINT sourceSuperpixelIndex)
{
    int n = superpixelNeighbors.Length();

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
            for(UINT neighborIndex = 0; neighborIndex < curEntry.n->indices.Length(); neighborIndex++)
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

void Recolorizer::ComputeNeighborWeights(const AppParameters &parameters, const Bitmap &bmp)
{
    ComponentTimer timer("Computing neighborhood weights");

    Console::WriteLine("Computing pixel weights");
    for(UINT y = 0; y < bmp.Height(); y++)
    {
        for(UINT x = 0; x < bmp.Width(); x++)
        {
            PixelNeighborhood &curPixel = pixelNeighborhoods(y, x);
            const UINT k = curPixel.indices.Length();

            ColorCoordinate curPixelCoordinate(parameters, bmp[y][x], Vec2i(x, y), bmp.Width(), bmp.Height());
            curPixel.weights = ComputeWeights(parameters, curPixel.indices, curPixelCoordinate.features);
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

void Recolorizer::VisualizeSourceDistance(const AppParameters &parameters, const Bitmap &bmp) const
{
    Bitmap result(bmp.Width(), bmp.Height());
    result.Clear(RGBColor::Black);

    double max = 2.0;
    //for(auto &s : superpixelNeighbors) max = Math::Max(max, s.shortestDist);

    for(UINT superpixelIndex = 0; superpixelIndex < superpixelColors.Length(); superpixelIndex++)
    {
        const ColorCoordinate &superpixelColor = superpixelColors[superpixelIndex];
        result[superpixelColor.coord.y][superpixelColor.coord.x] = RGBColor::Interpolate(RGBColor::Blue, RGBColor::Red, float(superpixelNeighbors[superpixelIndex].shortestDist / max));
    }
    result.SavePNG("../Results/sourceDistance.png");
}

void Recolorizer::VisualizeSuperpixels(const AppParameters &parameters, const Bitmap &bmp, const Vector<Vec3f> *newSuperpixelColors, const String &filename) const
{
    Bitmap result(bmp.Width(), bmp.Height());
    result.Clear(RGBColor::Black);

    for(UINT superpixelIndex = 0; superpixelIndex < superpixelColors.Length(); superpixelIndex++)
    {
        const ColorCoordinate &superpixel = superpixelColors[superpixelIndex];
        if(newSuperpixelColors == NULL)
            result[superpixel.coord.y][superpixel.coord.x] = superpixel.color;
        else
            result[superpixel.coord.y][superpixel.coord.x] = RGBColor((*newSuperpixelColors)[superpixelIndex]);
    }
    result.SavePNG("../Results/" + filename + ".png");
}

void Recolorizer::VisualizeNearestNeighbors(const AppParameters &parameters, const Bitmap &bmp) const
{
    const UINT debugPixelCount = 20;
    for(UINT debugPixelIndex = 0; debugPixelIndex < debugPixelCount; debugPixelIndex++)
    {
        int x = rand() % (bmp.Width() - 2) + 1;
        int y = rand() % (bmp.Height() - 2) + 1;

        const PixelNeighborhood &curPixel = pixelNeighborhoods(y, x);

        Bitmap result(bmp.Width(), bmp.Height());
        result.Clear(RGBColor::Black);

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
}
