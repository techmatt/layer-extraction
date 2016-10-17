
#include "main.h"

/*function<float(const vector<float> &)> LightingConstraints::makeFitnessFunction() const
{

}*/

void LightingConstraints::init(const Bitmap &startImage, float startWeight, const Bitmap &targetImage, float targetWeight)
{
	pixelConstraints.allocate(targetImage.getDimensions());
	for (auto &p : pixelConstraints)
	{
		const vec4uc s4 = startImage(p.x, p.y);
		const vec4uc t4 = targetImage(p.x, p.y);
		if (s4 == t4)
		{
			p.value.weight = startWeight;
			p.value.targetColor = LightUtil::toColorVec3(s4);
		}
		else
		{
			p.value.weight = targetWeight;
			p.value.targetColor = LightUtil::toColorVec3(t4);
		}
	}
}

double LightingConstraints::evalFitness(const ImageLayers & l, const vector<float>& x) const
{
	const Bitmap bmp = l.compositeImage(LightUtil::rawToLights(x));

	double fitness = 0.0;
	for (auto &p : pixelConstraints)
	{
		const vec3f cA = p.value.targetColor;
		const vec3f cB = LightUtil::toColorVec3(bmp(p.x, p.y));
		const float diff = vec3f::distSq(cA, cB);
		fitness -= diff * p.value.weight;
	}

	return fitness;
}

void LightingConstraints::saveDebug()
{
	Grid2<vec3f> g(pixelConstraints.getDimensions());
	for (auto &p : g)
	{
		p.value = pixelConstraints(p.x, p.y).targetColor;
	}
	Bitmap bmp = LightUtil::gridToBitmap(g);
	LodePNG::save(bmp, params().debugDir + "constraints.png");
}