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



bool NeighborhoodGenerator::Generate(const GaussianPyramid &pyramid, int level, int xCenter, int yCenter, double* result) const
{
	const int layerCount = pyramid.NumLayers();

	UINT dimensionIndex = 0;
	bool inBounds = true;

	const int width  = pyramid[level].First().Width();
	const int height = pyramid[level].First().Height();

	//Vec2i centerPt = pyramid.TransformCoordinates(Vec2i(xCenter, yCenter), 0, level);
	for(int layerIndex = 0; layerIndex < layerCount; layerIndex++)
	{
		for(int row = yCenter - _neighborhoodSize; row <= yCenter + _neighborhoodSize; row++)
		{
			for(int col = xCenter - _neighborhoodSize; col <= xCenter + _neighborhoodSize; col++)
			{
				if(row < 0 || row >= height || col < 0 || col >= width)
				{
					inBounds = false;
					result[dimensionIndex++] = 0;
				} else
					result[dimensionIndex++] = pyramid[level][layerIndex].pixelWeights(row,col);
			}
		}
	}
	return inBounds;
}

bool NeighborhoodGenerator::Generate(const GaussianPyramid &pyramid, const Grid<Vec2i> &coordinates, int level, int width, int height, int xCenter, int yCenter, double* result) const
{
	const int layerCount = pyramid.NumLayers();

	UINT dimensionIndex = 0;
	bool inBounds = true;

	//Vec2i centerPt = pyramid.TransformCoordinates(Vec2i(xCenter, yCenter), 0, level);
	for(int layerIndex = 0; layerIndex < layerCount; layerIndex++)
	{
		for(int row = yCenter - _neighborhoodSize; row <= yCenter + _neighborhoodSize; row++)
		{
			for(int col = xCenter - _neighborhoodSize; col <= xCenter + _neighborhoodSize; col++)
			{
				if(row < 0 || row >= height || col < 0 || col >= width)
				{
					inBounds = false;
					result[dimensionIndex++] = 0;
				} else {
					Assert( coordinates(row,col).x >= 0 &&  coordinates(row,col).x < pyramid[level].First().Width() &&  coordinates(row,col).y >= 0 &&  coordinates(row,col).y < pyramid[level].First().Height(),
						"coordinates out of bounds");
					result[dimensionIndex++] = pyramid[level][layerIndex].pixelWeights(coordinates(row,col).y, coordinates(row,col).x);
				}
			}
		}
	}
	return inBounds;
}

//// synthesis by layer
bool NeighborhoodGenerator::Generate(const PixelLayerSet &layers, const Vector<int> &order, int iteration, int xCenter, int yCenter, double* result) const
{
	const int width  = layers.First().Width();
	const int height = layers.First().Height();

	UINT dimensionIndex = 0;
	bool inBounds = true;

	//Vec2i centerPt = pyramid.TransformCoordinates(Vec2i(xCenter, yCenter), 0, level);
	for(int i = 0; i <= iteration; i++)
	{
		int layerIndex = order[i];
		for(int row = yCenter - _neighborhoodSize; row <= yCenter + _neighborhoodSize; row++)
		{
			for(int col = xCenter - _neighborhoodSize; col <= xCenter + _neighborhoodSize; col++)
			{
				if(row < 0 || row >= height || col < 0 || col >= width)
				{
					inBounds = false;
					result[dimensionIndex++] = 0;
				} else {
					result[dimensionIndex++] = layers[layerIndex].pixelWeights(row, col);
				}
			}
		}
	}
	return inBounds;
}

bool NeighborhoodGenerator::Generate(const PixelLayerSet &synthlayers, const Vector< Grid<Vec2i> > &coordinateset, Vector<int> order, int iteration,
									 int width, int height, int xCenter, int yCenter, double* result) const
{
	UINT dimensionIndex = 0;
	bool inBounds = true;

	for (int i = 0; i <= iteration; i++) {
		for(int row = yCenter - _neighborhoodSize; row <= yCenter + _neighborhoodSize; row++)
		{
			for(int col = xCenter - _neighborhoodSize; col <= xCenter + _neighborhoodSize; col++)
			{
				if(row < 0 || row >= height || col < 0 || col >= width)
				{
					inBounds = false;
					result[dimensionIndex++] = 0;
				} else {
					Assert( col >= 0 &&  col < synthlayers[i].Width() &&  row >= 0 &&  row < synthlayers[i].Height(),
						"coordinates out of bounds");
					result[dimensionIndex++] = synthlayers[i].pixelWeights(row,col);
				}
			}
		}
	}
	return inBounds;
}



//// synthesis by layer w/ extra features
bool NeighborhoodGenerator::Generate(const Vector<PixelLayerSet> &layerfeatures, const Vector<int> &order, int iteration, int xCenter, int yCenter, double* result) const
{
	const int width  = layerfeatures.First().First().Width();
	const int height = layerfeatures.First().First().Height();

	UINT dimensionIndex = 0;
	bool inBounds = true;

	//Vec2i centerPt = pyramid.TransformCoordinates(Vec2i(xCenter, yCenter), 0, level);
	for(int layeridx = 0; layeridx <= iteration; layeridx++)
	{
		int layerIndex = order[layeridx];
		for (int feature = 0; feature < layerfeatures[layerIndex].Length(); feature++) {

			for(int row = yCenter - _neighborhoodSize; row <= yCenter + _neighborhoodSize; row++)
			{
				for(int col = xCenter - _neighborhoodSize; col <= xCenter + _neighborhoodSize; col++)
				{
					if(row < 0 || row >= height || col < 0 || col >= width)
					{
						inBounds = false;
						result[dimensionIndex++] = 0;
					} else {
						result[dimensionIndex++] = layerfeatures[layerIndex][feature].pixelWeights(row, col);
					}
				}
			}
		}
	}
	return inBounds;
}

bool NeighborhoodGenerator::Generate(const Vector<PixelLayerSet> &layerfeatures, const Vector< Grid<Vec2i> > &coordinateset, Vector<int> order, int iteration,
									 int width, int height, int xCenter, int yCenter, double* result) const
{
	UINT dimensionIndex = 0;
	bool inBounds = true;

	for (int iter = 0; iter <= iteration; iter++) {
		int layerIndex = order[iter];
		for (int feature = 0; feature < layerfeatures[layerIndex].Length(); feature++) {

			for(int row = yCenter - _neighborhoodSize; row <= yCenter + _neighborhoodSize; row++)
			{
				for(int col = xCenter - _neighborhoodSize; col <= xCenter + _neighborhoodSize; col++)
				{
					if(row < 0 || row >= height || col < 0 || col >= width)
					{
						inBounds = false;
						result[dimensionIndex++] = 0;
					} else {
						Assert( coordinateset[iter](row,col).x >= 0 && coordinateset[iter](row,col).x < layerfeatures.First().First().Width() && 
							coordinateset[iter](row,col).y >= 0 && coordinateset[iter](row,col).y < layerfeatures.First().First().Height(),
							"coordinates out of bounds");
						result[dimensionIndex++] = layerfeatures[layerIndex][feature].pixelWeights(coordinateset[iter](row,col).y, coordinateset[iter](row,col).x);
					}
				}
			}
		}
	}
	return inBounds;
}