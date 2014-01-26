#include "Main.h"
#include "RecolorizerPixel.h"


RecolorizerPixel::RecolorizerPixel(void)
{
}


RecolorizerPixel::~RecolorizerPixel(void)
{
	pixelColors.FreeMemory();
}

void RecolorizerPixel::Init(const AppParameters &parameters, const Bitmap &bmp)
{
	ComputeNearestNeighbors(parameters, bmp);
	ComputeNeighborWeights(parameters, bmp);	
	ComputeWeightMatrix(parameters, bmp);
	VisualizeNearestNeighbors(parameters, bmp);
	TestNeighborWeights(parameters, bmp);
}
    
Bitmap RecolorizerPixel::Recolor(const AppParameters &parameters, const Bitmap &bmp, const Vector<PixelConstraint> &targetPixelColors)
{
	    //ComponentTimer timer("Computing weight matrix");
    const UINT n = pixelColors.Length();
    SparseMatrix<double> W(n, n);
	

    for(UINT pixelIndex = 0; pixelIndex < n; pixelIndex++)
    {
        W.PushDiagonalElement(pixelIndex, 1.0);
		int y = pixelIndex / bmp.Width();
		int x = pixelIndex % bmp.Width();
        const PixelNeighborhood &neighborhood = pixelNeighborhoods(y,x);
		double sum = 0;
        for(UINT neighborIndex = 0; neighborIndex < neighborhood.indices.Length(); neighborIndex++)
        {
            W.PushElement(pixelIndex, neighborhood.indices[neighborIndex], -neighborhood.weights[neighborIndex]);
			sum += neighborhood.weights[neighborIndex];
        }
		if (sum < 0.9 || sum > 1.01)
			Console::WriteLine("Sum ="+String(sum));
    }

    Console::WriteLine("Building W-matrix");
    //SparseMatrix<double> WMatrix = W.Transpose()*W;
	Console::WriteLine("Done building");

	WBase = W;//WMatrix;



	ComponentTimer timer("Recoloring");

    //const UINT n = pixelColors.Length();


    Console::WriteLine("Loading Eigen matrix...");
	SparseMatrix<double> M(2*n,n);
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
        //M.PushDiagonalElement(index, parameters.userConstraintWeight);
		M.PushElement(n+index, index, sqrt(parameters.userConstraintWeight)); 
    }

	EigenSolver solver;

    Console::WriteLine("Starting solve...");

    Vector<Vec3f> newPixelColors(n);

	bool useMatlab = parameters.useMatlab;

	if (!useMatlab)
	{
		for(UINT featureIndex = 0; featureIndex < 3; featureIndex++)
		{
			ComponentTimer("Linear solve " + String(featureIndex));
        
			Vector<double> b(2*n,0); //[0 G]
			for(const PixelConstraint &c : targetPixelColors)
			{
				int index = n + c.coord.y*bmp.Width()+c.coord.x;
				//b[index] += parameters.userConstraintWeight * c.targetColor[featureIndex];
				b[index] += sqrt(parameters.userConstraintWeight) * c.targetColor[featureIndex];
			}

			//Vector<double> x = solver.Solve(M, b, EigenSolver::ConjugateGradient_Diag);
			Vector<double> x = solver.Solve(M, b, EigenSolver::QR);
			Console::WriteLine("X length "+String(x.Length()));

			for(UINT i = 0; i < n; i++)
			{
				newPixelColors[i][featureIndex] = (float)x[i];
			}
		}
	} else
	{
		
		Console::WriteLine("Dumping to Matlab");
		ofstream QFile("../Matlab/M.txt");
		PersistentAssert(!QFile.fail(), "Failed to open file");

		for (int r=0; r < 2*n; r++)
		{
			const SparseRow<double> &row = M.Rows()[r];
			for (int c=0; c<row.Data.Length(); c++)
			{
				double val = row.Data[c].Entry;
				double col = row.Data[c].Col;
				if (val != 0)
					QFile << String(r+1) << " " << String(col+1) << " " << String(val) << endl;
			}
		}
		QFile.close();

		for(UINT featureIndex = 0; featureIndex < 3; featureIndex++)
		{
			Console::WriteLine("Dumping...");
        
			Vector<double> b(2*n,0); //[0 G]
			for(const PixelConstraint &c : targetPixelColors)
			{
				int index = n + c.coord.y*bmp.Width()+c.coord.x;
				//b[index] += parameters.userConstraintWeight * c.targetColor[featureIndex];
				b[index] += sqrt(parameters.userConstraintWeight) * c.targetColor[featureIndex];
			}
			String outb = "../Matlab/rb"+String(featureIndex+1)+".txt";
			ofstream bFile(outb.CString());
			PersistentAssert(!bFile.fail(), "Failed to open file");
			for (int r=0; r<b.Length(); r++)
				bFile << String(b[r]) << endl;
			bFile.close();
		}

		Console::WriteLine("Done dumping matrix files");
		Console::WriteLine("Waiting for Matlab...Press num pad 0 when done");
		while(GetAsyncKeyState(VK_NUMPAD0) == 0)
		{
			Sleep(1000);
		}

		for(UINT featureIndex = 0; featureIndex < 3; featureIndex++)
		{
			Console::WriteLine("Reading...");
        
			Vector<double> x;

			String inx = "../Matlab/rx"+String(featureIndex+1)+".txt";
			Vector<String> xLines = Utility::GetFileLines(inx);
			for (String line:xLines)
				x.PushEnd(line.ConvertToDouble());

			for (UINT i=0; i<n; i++)
				newPixelColors[i][featureIndex] = (float)x[i];

		}
		Console::WriteLine("Done!");

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
    /*ComponentTimer timer("Computing weight matrix");
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
    //SparseMatrix<double> WMatrix = W.Transpose()*W;
	Console::WriteLine("Done building");

	WBase = W;//WMatrix;*/
}

void RecolorizerPixel::TestNeighborWeights(const AppParameters &parameters, const Bitmap &bmp) const
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
                sum += Vec3f(pixelColors[curPixel.indices[neighborIndex]].color) * (float)curPixel.weights[neighborIndex];
            }

            float error = Vec3f::Dist(Vec3f(bmp[y][x]), sum); 
			if (error > 0.01)
				Console::WriteLine("e=" + String(error));
			double msum = curPixel.weights.Sum();
			if (msum < 0.9 || msum > 1.01)
				Console::WriteLine("sum= " + String(msum));
        }
    }
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
			float* features = new float[5];
			for (int d=0; d<5; d++)
				features[d] = c.features[d];
			points.PushEnd(features);
		}
	}

	/*for (int i=0; i<10; i++)
	{
		int x = rand() % (bmp.Width() - 2) + 1;
        int y = rand() % (bmp.Height() - 2) + 1;
		int index = y*bmp.Width()+x;
		Console::WriteLine("x "+String(x) + " y "+ String(y)+"\n");
		for (int d=0; d<5; d++)
			Console::WriteString(String(points[index][d]) + " "); 
		Console::WriteLine("\n");
	}*/

    tree.BuildTree(points, 5, K + 1);

    pixelNeighborhoods.Allocate(bmp.Height(), bmp.Width());

    for(UINT y = 0; y < bmp.Height(); y++)
    {
        for(UINT x = 0; x < bmp.Width(); x++)
        {
			int index = y*bmp.Width()+x;
            ColorCoordinate curCoord(parameters, bmp[y][x], Vec2i(x, y), bmp.Width(), bmp.Height());
            tree.KNearest(curCoord.features, K+1, pixelNeighborhoods(y, x).indices, 0.0f);

			PixelNeighborhood& n = pixelNeighborhoods(y,x);
			if(n.indices.Contains(index))
			{
				n.indices.RemoveSwap(n.indices.FindFirstIndex(index));
			}
			else
			{
				n.indices.PopEnd();
			}
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
	/*DenseMatrix<double> z(k, 5);
    for(UINT neighborIndex = 0; neighborIndex < k; neighborIndex++)
    {
        const ColorCoordinate &neighbor = pixelColors[indices[neighborIndex]];
        for(UINT dimensionIndex = 0; dimensionIndex < 5; dimensionIndex++)
        {
            z(neighborIndex, dimensionIndex) = neighbor.features[dimensionIndex] - pixelFeatures[dimensionIndex];
        }
    }*/

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

void RecolorizerPixel::VisualizeNearestNeighbors(const AppParameters &parameters, const Bitmap &bmp) const
{
    const UINT debugPixelCount = 10;
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
            const ColorCoordinate &pixel = pixelColors[neighborIndex];
            result[pixel.coord.y][pixel.coord.x] = pixel.color;
        }

        result.SavePNG("../Results/pixel" + String(debugPixelIndex) + ".png");
    }
}
