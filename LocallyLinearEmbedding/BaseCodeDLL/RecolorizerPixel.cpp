#include "Main.h"
#include "RecolorizerPixel.h"


RecolorizerPixel::RecolorizerPixel(void)
{
}


RecolorizerPixel::~RecolorizerPixel(void)
{
}

void RecolorizerPixel::Init(const AppParameters &parameters, const Bitmap &bmp)
{
	ComputeNearestNeighbors(parameters, bmp);
	ComputeNeighborWeights(parameters, bmp);	
	ComputeWeightMatrix(parameters, bmp);
}
    
Bitmap RecolorizerPixel::Recolor(const AppParameters &parameters, const Bitmap &bmp, const Vector<PixelConstraint> &targetPixelColors)
{
	ComponentTimer timer("Recoloring");

    const UINT n = pixelColors.Length();


    Console::WriteLine("Loading Eigen matrix...");
	SparseMatrix<double> M(n,n);
	for (UINT pixelIndex = 0; pixelIndex < n; pixelIndex++)
	{
		const SparseRow<double> &curRow = WBase[pixelIndex];
		for(const SparseElement<double> &e : curRow.Data)
		{
			M.PushElement(pixelIndex, e.Col, e.Entry);
		}
	}
	
	//add the constraint weights (LHS)
    for(const PixelConstraint &c : targetPixelColors)
    {
		int index = c.coord.y*bmp.Width()+c.coord.x;
        M.PushDiagonalElement(index, parameters.userConstraintWeight);
    }

	EigenSolver solver;

    Console::WriteLine("Starting sove...");

    Vector<Vec3f> newPixelColors(n);

    for(UINT featureIndex = 0; featureIndex < 3; featureIndex++)
    {
        ComponentTimer("Linear solve " + String(featureIndex));
        
        Vector<double> b(n,0);
        for(const PixelConstraint &c : targetPixelColors)
        {
			int index = c.coord.y*bmp.Width()+c.coord.x;
            b[index] += parameters.userConstraintWeight * c.targetColor[featureIndex];
        }

        Vector<double> x = solver.Solve(M, b, EigenSolver::ConjugateGradient_Diag);
        
        for(UINT i = 0; i < n; i++)
        {
            newPixelColors[i][featureIndex] = (float)x[i];
        }
    }

	Bitmap result(bmp.Width(), bmp.Height());
	for (int y=0; y<bmp.Height(); y++)
	{
		for (int x=0; x<bmp.Width(); x++)
		{
			int index = y*bmp.Width()+x;
			result[y][x] = RGBColor(newPixelColors[index]);
		}
	}

	return result;
}


void RecolorizerPixel::ComputeWeightMatrix(const AppParameters &parameters, const Bitmap &bmp)
{
    ComponentTimer timer("Computing weight matrix");
    const UINT n = pixelColors.Length();
    SparseMatrix<double> W(n, n);

    for(UINT pixelIndex = 0; pixelIndex < n; pixelIndex++)
    {
        W.PushDiagonalElement(pixelIndex, 1.0);
		int y = pixelIndex / bmp.Width();
		int x = pixelIndex % bmp.Width();
        const PixelNeighborhood &neighborhood = pixelNeighborhoods(y,x);
        for(UINT neighborIndex = 0; neighborIndex < neighborhood.indices.Length(); neighborIndex++)
        {
            W.PushElement(pixelIndex, neighborhood.indices[neighborIndex], -neighborhood.weights[neighborIndex]);
        }
    }

    Console::WriteLine("Building W-matrix");
    SparseMatrix<double> WMatrix = W.Transpose() * W;
	Console::WriteLine("Done building");

	WBase = WMatrix;
}

void RecolorizerPixel::ComputeNearestNeighbors(const AppParameters &parameters, const Bitmap &bmp)
{
    ComponentTimer timer("Computing nearest neighbors");

	int K = parameters.recolorizerPixelNeighbors;

    KDTree tree;

    Vector<const float*> points;
	for (int y=0; y<bmp.Height(); y++)
	{
		for (int x=0; x<bmp.Width(); x++)
		{
			ColorCoordinate c(parameters, bmp[y][x], Vec2i(x,y), bmp.Width(), bmp.Height());
			pixelColors.PushEnd(c);
			points.PushEnd(c.features);
		}
	}

    tree.BuildTree(points, 5, K + 1);

    pixelNeighborhoods.Allocate(bmp.Height(), bmp.Width());

    for(UINT y = 0; y < bmp.Height(); y++)
    {
        for(UINT x = 0; x < bmp.Width(); x++)
        {
            ColorCoordinate curCoord(parameters, bmp[y][x], Vec2i(x, y), bmp.Width(), bmp.Height());
            tree.KNearest(curCoord.features, K, pixelNeighborhoods(y, x).indices, 0.0f);
        }
    }

}

void RecolorizerPixel::ComputeNeighborWeights(const AppParameters &parameters, const Bitmap &bmp)
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
}

Vector<double> RecolorizerPixel::ComputeWeights(const AppParameters &parameters, const Vector<UINT> &indices, const float *pixelFeatures)
{
	 //
    // Remember that for forming linear combinations, we consider only the color terms (3 dimensions) and not the spatial ones!
    //

    const UINT k = indices.Length();
    DenseMatrix<double> z(k, 3);
    for(UINT neighborIndex = 0; neighborIndex < k; neighborIndex++)
    {
        const ColorCoordinate &neighbor = pixelColors[indices[neighborIndex]];
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
