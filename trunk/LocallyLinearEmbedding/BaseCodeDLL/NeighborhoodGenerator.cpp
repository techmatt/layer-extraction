#include "Main.h"
#include "NeighborhoodGenerator.h"


NeighborhoodGenerator::NeighborhoodGenerator(UINT neighborhoodSize, UINT numLayers, UINT depth)
{
	_neighborhoodSize = neighborhoodSize;
	_dimension = Math::Square(neighborhoodSize*2+1)*numLayers*depth;
}


NeighborhoodGenerator::~NeighborhoodGenerator(void)
{
}

/*bool NeighborhoodGenerator::Generate(const PixelLayerSet &layers, int xCenter, int yCenter, double* result) const
{
    const UINT width  = layers.First().pixelWeights.Cols();
    const UINT height = layers.First().pixelWeights.Rows();
	const UINT layerCount = layers.Length();

    UINT dimensionIndex = 0;
	bool inBounds = true;

	for(UINT layerIndex = 0; layerIndex < layerCount; layerIndex++)
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
}*/

bool NeighborhoodGenerator::Generate(const GaussianPyramid &pyramid, int xCenter, int yCenter, double* result) const
{
	const int layerCount = pyramid.NumLayers();
	const int depth = pyramid.Depth();

    UINT dimensionIndex = 0;
	bool inBounds = true;

	for (int depthIndex = 0; depthIndex < depth; depthIndex++)
	{
		const int width  = pyramid[depthIndex].First().Width();
		const int height = pyramid[depthIndex].First().Height();

		Vec2i centerPt = pyramid.TransformCoordinates(Vec2i(xCenter, yCenter),0,depthIndex);

		for(int layerIndex = 0; layerIndex < layerCount; layerIndex++)
		{
			for(int y = centerPt.y - _neighborhoodSize; y <= centerPt.y + _neighborhoodSize; y++)
			{
				for(int x = centerPt.x - _neighborhoodSize; x <= centerPt.x + _neighborhoodSize; x++)
				{
					if(y < 0 || y >= height || x < 0 || x >= width)
					{
						inBounds = false;
						result[dimensionIndex++] = 0;
					} else
						result[dimensionIndex++] = pyramid[depthIndex][layerIndex].pixelWeights(y,x);
				}
			}
		}
	}
    return inBounds;
}
