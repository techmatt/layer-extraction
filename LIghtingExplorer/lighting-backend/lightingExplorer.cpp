
#include "main.h"

const float minDistSqThreshold = 1.0f;

void LightingExplorer::init()
{
	fullLayers.loadCSV(R"(C:\Code\layer-extraction\Images\les-miserables-layers\)");
	smallLayers = fullLayers;
	smallLayers.downSample(Constants::smallLayersBlockSize);

	blocksX = fullLayers.dimX / Constants::signatureBlockSize;
	blocksY = fullLayers.dimY / Constants::signatureBlockSize;
	signatureDim = blocksX * blocksY * 3;
	cout << "signatude dim: " << signatureDim << endl;
}

void LightingExplorer::populateCandidates(const LightingConstraints &constraints)
{
	candidateSamples.clear();
	
	GradientFreeProblem problem;
	problem.fitness = FitnessFunction([&](const vector<float> &x) {
		return constraints.evalFitness(smallLayers, x);
	});
	problem.render = RenderFunction([&](const vector<float> &x) {
		return fullLayers.compositeImage(LightUtil::rawToLights(x));
	});

	const bool useGoodStart = true;
	vector<float> startX;
	for (auto &l : fullLayers.layers)
	{
		if (useGoodStart)
		{
			startX.push_back(l.baseColor.x);
			startX.push_back(l.baseColor.y);
			startX.push_back(l.baseColor.z);
		}
		else
		{
			startX.push_back(0.0f);
			startX.push_back(0.0f);
			startX.push_back(0.0f);
		}
	}
	problem.startingPoints.push_back(startX);

	//GradientFreeOptRandomWalk opt;
	GradientFreeOptCMAES opt;
	vector<float> result = opt.optimize(problem);

	LightingSample sample;
	sample.lightColors = LightUtil::rawToLights(result);
	sample.signature = makeSignature(sample.lightColors);
	candidateSamples.push_back(sample);
}

/*void LightingExplorer::go()
{
	vector<vec3f> colors;
	for (auto &l : layers.layers)
		colors.push_back(l.baseColor);

	Bitmap bmp = layers.compositeImage(colors);
	LodePNG::save(bmp, R"(C:\Code\layer-extraction\Images\test.png)");
}*/

/*void LightingExplorer::maybeAddSample(const Bitmap &bmp, const vector<vec3f> &lightColors)
{
	auto signature = makeSignature(bmp);
	auto neighbors = hash.findSimilar(signature);
	for (auto &nIndex : neighbors)
	{
		const LightingSample &n = *samples[nIndex];
		const float distSq = math::distSqL2(signature, n.signature);
		if (distSq < minDistSqThreshold)
		{
			return;
		}
	}
	LightingSample *newSample = new LightingSample;
	newSample->lightColors = lightColors;
	newSample->signature = std::move(signature);
	hash.insert(newSample->signature, samples.size());
	samples.push_back(newSample);
}*/

vector<float> LightingExplorer::makeSignature(const vector<vec3f> &lights) const
{
	const Bitmap bmp = fullLayers.compositeImage(lights);
	return makeSignature(bmp);
}

vector<float> LightingExplorer::makeSignature(const Bitmap &bmp) const
{
	vector<float> result(signatureDim);
	int signatureIndex = 0;
	for (int blockY = 0; blockY < blocksY; blockY++)
		for (int blockX = 0; blockX < blocksX; blockX++)
		{
			vec3f avg = vec3f::origin;
			for(int yOffset = 0; yOffset < Constants::signatureBlockSize; yOffset++)
				for (int xOffset = 0; xOffset < Constants::signatureBlockSize; xOffset++)
				{
					const vec4uc v = bmp(blockX * Constants::signatureBlockSize + xOffset, blockY * Constants::signatureBlockSize + yOffset);
					avg.x += v.x;
					avg.y += v.y;
					avg.z += v.z;
				}
			avg /= Constants::signatureBlockSize * Constants::signatureBlockSize * 255.0f * 3.0f;
			result[signatureIndex++] = avg.x;
			result[signatureIndex++] = avg.y;
			result[signatureIndex++] = avg.z;
		}
	MLIB_ASSERT_STR(result.size() == signatureDim, "signature size mismatch");
	return result;
}
