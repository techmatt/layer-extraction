
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

double LightingConstraints::evalFitness(const Bitmap & bmp, const vector<float>& x) const
{
	double fitness = 0.0;
	for (auto &p : pixelConstraints)
	{
		const vec3f cA = p.value.targetColor;
		const vec3f cB = LightUtil::toColorVec3(bmp(p.x, p.y));
		const float diff = vec3f::distSq(cA, cB);

		//if (p.value.weight < 0.1f)
		if(false)
		{
			// hack: match value only
			const float vA = 0.3f * cA.x + 0.5f * cA.y + 0.2f * cA.z;
			const float vB = 0.3f * cB.x + 0.5f * cB.y + 0.2f * cB.z;
			const float diff = vA - vB;
			fitness -= diff * diff * p.value.weight;
		}
		else
		{
			fitness -= diff * p.value.weight;
		}
	}

	return fitness;
}

double LightingConstraints::evalFitness(const ImageLayers & l, const vector<float>& x) const
{
	const Bitmap bmp = l.compositeImage(LightUtil::rawToLights(x));
	return evalFitness(bmp, x);
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