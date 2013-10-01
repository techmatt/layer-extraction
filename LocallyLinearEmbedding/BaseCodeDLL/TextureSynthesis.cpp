#include "Main.h"



void TextureSynthesis::Init(const PixelLayerSet &exemplar, const NeighborhoodGenerator &generator, UINT nlevels, UINT reducedDimension)
{
	//TSGaussianPyramid g(nlevels, exemplar);
	//g.WritePyramid("debug/pyr");
    //InitPCA(exemplar, generator);
    //InitKDTree(exemplar, generator, reducedDimension);
}

/*void TextureSynthesis::InitPCA(const PixelLayerSet &exemplar, const NeighborhoodGenerator &generator)
{
    Console::WriteLine("Initializing PCA...");
    
    const UINT dimension = generator.Dimension();
	const UINT width = exemplar.First().pixelWeights.Cols();
	const UINT height = exemplar.First().pixelWeights.Rows();

    const UINT neighborhoodCount = width * height;
    Vector<const double*> neighborhoods(neighborhoodCount);

    for(UINT neighborhoodIndex = 0; neighborhoodIndex < neighborhoodCount; neighborhoodIndex++)
    {
        double *curNeighborhood = new double[dimension];
        bool success = false;
        while(!success)
        {
			int xCenter = rand() % width;
			int yCenter = rand() % height;
            success = generator.Generate(layers, xCenter, yCenter, curNeighborhood);
			for(UINT i = 0; i < dimension; i++)
			{
				if(curNeighborhood[i] < -10000.0 || curNeighborhood[i] > 10000.0)
				{
					success = generator.Generate(layers, xCenter, yCenter, curNeighborhood);
				}
			}
        }
        neighborhoods[neighborhoodIndex] = curNeighborhood;
    }

    _pca.InitFromDensePoints(neighborhoods, dimension);
    neighborhoods.DeleteMemory();
}

void TextureSynthesis::InitKDTree(const PixelLayerSet &layers, const NeighborhoodGenerator &generator, UINT reducedDimension)
{
	_reducedDimension = reducedDimension;

	const UINT width = layers.First().pixelWeights.Cols();
	const UINT height = layers.First().pixelWeights.Rows();
    const UINT dimension = generator.Dimension();
	const UINT neighbors = 5;

    Vector<const double*> allNeighborhoods;
    
    double *neighborhood = new double[dimension];
    for(UINT y = 0; y < height; y++)
    {
        for(UINT x = 0; x < width; x++)
        {
            if(generator.Generate(layers, x, y, neighborhood))
            {
                double *reducedNeighborhood = new double[reducedDimension];
                _pca.Transform(reducedNeighborhood, neighborhood, reducedDimension);
                allNeighborhoods.PushEnd(reducedNeighborhood);
                _treeCoordinates.PushEnd(Vec2i(x, y));
            }
        }
    }
    delete[] neighborhood;
    Console::WriteLine(String("Building KDTree, ") + String(allNeighborhoods.Length()) + String(" neighborhoods..."));
    _tree.BuildTree(allNeighborhoods, reducedDimension, neighbors);
    allNeighborhoods.DeleteMemory();
}
*/