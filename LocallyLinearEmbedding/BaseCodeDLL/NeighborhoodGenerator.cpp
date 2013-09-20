#include "Main.h"
#include "NeighborhoodGenerator.h"


NeighborhoodGenerator::NeighborhoodGenerator(UINT neighborhoodSize, UINT numLayers)
{
	_neighborhoodSize = neighborhoodSize;
	_dimension = Math::Square(neighborhoodSize*2+1)*numLayers;
}


NeighborhoodGenerator::~NeighborhoodGenerator(void)
{
}

bool NeighborhoodGenerator::Generate(const PixelLayerSet &layers, int xCenter, int yCenter, double* result) const
{
    const UINT width  = layers.First().pixelWeights.Cols();
    const UINT height = layers.First().pixelWeights.Rows();
	const UINT layerCount = layers.Length();

    UINT dimensionIndex = 0;
	bool inBounds = true;

	for(int layerIndex = 0; layerIndex < layerCount; layerIndex++)
	{
		for(int y = yCenter - _neighborhoodSize; y <= yCenter + _neighborhoodSize; y++)
		{
			for(int x = xCenter - _neighborhoodSize; x <= xCenter + _neighborhoodSize; x++)
			{
				if(y < 0 || y >= height || x < 0 || x >= width)
				{
					inBounds = false;
					result[dimensionIndex++] = 0;
				} else
					result[dimensionIndex++] = layers[layerIndex].pixelWeights(y,x);
			}
		}
	}
    return inBounds;
}
