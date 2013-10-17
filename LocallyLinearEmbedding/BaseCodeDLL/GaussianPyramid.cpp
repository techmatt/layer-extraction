#include "Main.h"
#include "GaussianPyramid.h"

GaussianPyramid::GaussianPyramid(const PixelLayerSet &original, int depth, double alpha)
{
	Init(original, depth, alpha);
}

void GaussianPyramid::Init(const PixelLayerSet &original, int depth, double alpha)
{
	//Make a Gaussian pyramid of the number of specified depth
	//The first level in the pyramid is the original set of layers, each successive level is coarser...

	Filter filter(alpha);

	_data.FreeMemory();
	_data.PushEnd(PixelLayerSet(original));

	int width = original.First().Width();
	int height = original.First().Height();

	for (int depthIndex=1; depthIndex<depth; depthIndex++)
	{
		//figure out dimensions for the next coarsest level
		PixelLayerSet level;

		width = (width+1)/2;
		height = (height+1)/2;

		for (UINT layerIndex=0; layerIndex<original.Length(); layerIndex++)
		{
			PixelLayer layer(original[layerIndex].color, width, height);

			for (int x=0; x<width; x++)
				for (int y=0; y<height; y++)
					layer.pixelWeights(y,x) = ApplyFilter(Vec2i(2*x, 2*y), filter, _data[depthIndex-1][layerIndex]);

			level.PushEnd(layer);
		}
		_data.PushEnd(level);

		if (width == 1 && height == 1)
			break;
	}

}

double GaussianPyramid::ApplyFilter(Vec2i point, Filter filter, const PixelLayer &layer)
{
	int radius = filter.Radius();

	double result = 0;
	double sumWeights = 0;
	for (int dx=-2; dx<=2; dx++)
	{
		for (int dy=-2; dy<=2; dy++)
		{
			Vec2i neighbor = point + Vec2i(dx,dy);
			if (layer.pixelWeights.ValidCoordinates(neighbor.y, neighbor.x))
			{
				result += filter(dx,dy)*layer.pixelWeights(neighbor.y, neighbor.x);
				sumWeights += filter(dx,dy);
			}
		}
	}

	return result/sumWeights;
}

Vec2i GaussianPyramid::TransformCoordinates(Vec2i point, int fromDepth, int toDepth) const
{
	int delta = Math::AbsInt(toDepth-fromDepth);
	if (toDepth > fromDepth)
	{
		return point/(int)pow(2,delta);

	} else
	{
		return point*(int)pow(2,delta);
	} 
}