
#include "main.h"

const float minDistSqThreshold = 1.0f;

void LightingExplorer::init()
{
	//fullLayers.loadCSV(R"(C:\Code\layer-extraction\Images\les-miserables-layers\)");
	fullLayers.loadDAT(R"(C:\Code\layer-extraction\Images\les-miserables-layers\)");
	smallLayers = fullLayers;
	smallLayers.downSample(Constants::smallLayersBlockSize);

	blocksX = smallLayers.dimX / Constants::signatureBlockSize;
	blocksY = smallLayers.dimY / Constants::signatureBlockSize;
	signatureDim = blocksX * blocksY * 3;
	cout << "signatude dim: " << signatureDim << endl;
}

void LightingExplorer::populateCandidates(const LightingConstraints &constraints)
{
	candidateSamples.clear();
	acceptedSamples.clear();
	
	const bool useGoodStart = false;
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
	
	const int finalSampleCount = 20;
	const int exclusionIterCount = 5;
	for (int iter = 0; iter < exclusionIterCount; iter++)
	{
		populateCandidatesExclusion(constraints, startX, acceptedSamples);

		LightingSample *bestSample = nullptr;
		double bestFitness = -numeric_limits<double>::max();
		for (auto &c : candidateSamples)
		{
			if (c.baseFitness > bestFitness)
			{
				bool acceptable = true;
				for (auto &e : acceptedSamples)
				{
					const float dist = math::distL2(c.signature, e.signature) * Constants::signatureDistScale / signatureDim;
					//cout << dist << "," << c.baseFitness << "," << bestFitness << endl;
					if (dist < Constants::acceptanceExclusionRadius)
					{
						const bool verbose = false;
						if (verbose)
						{
							const Bitmap b0 = fullLayers.compositeImage(c.lightColors);
							const Bitmap b1 = fullLayers.compositeImage(e.lightColors);
							LodePNG::save(b0, params().debugDir + "b0.png");
							LodePNG::save(b1, params().debugDir + "b1.png");
							cout << dist << endl;
							cin.get();
						}
						acceptable = false;
					}
				}
				if (acceptable)
				{
					bestSample = &c;
					bestFitness = c.baseFitness;
				}
			}
		}
		if (bestSample == nullptr)
		{
			cout << "No acceptable samples found" << endl;
		}
		else
		{
			cout << "Accepted sample fitness: " << bestFitness << endl;
			acceptedSamples.push_back(*bestSample);
		}
	}

	for (auto &a : iterate(acceptedSamples))
	{
		cout << a.index << ": " << a.value.baseFitness << endl;
		LodePNG::save(fullLayers.compositeImage(a.value.lightColors), params().debugDir + "accepted" + to_string(a.index) + ".png");
	}

	for (int iter = 0; iter < finalSampleCount; iter++)
	{
		LightingSample *bestSample = nullptr;
		double bestFitness = -numeric_limits<double>::max();
		for (auto &c : candidateSamples)
		{
			if (c.baseFitness > bestFitness)
			{
				bool acceptable = true;
				for (auto &e : finalSamples)
				{
					const float dist = math::distL2(c.signature, e.signature) * Constants::signatureDistScale / signatureDim;
					//cout << dist << "," << c.baseFitness << "," << bestFitness << endl;
					if (dist < Constants::finalExclusionRadius)
					{
						acceptable = false;
					}
				}
				if (acceptable)
				{
					bestSample = &c;
					bestFitness = c.baseFitness;
				}
			}
		}
		if (bestSample == nullptr)
		{
			cout << "No acceptable samples found" << endl;
			finalSamples.push_back(util::randomElement(candidateSamples));
		}
		else
		{
			cout << "Final sample fitness: " << bestFitness << endl;
			finalSamples.push_back(*bestSample);
		}
	}

	const int displayX = 5;
	const int displayY = 4;
	const int buffer = 10;
	int resultIndex = 0;
	Bitmap bmpOut((fullLayers.dimX + buffer) * displayX, (fullLayers.dimY + buffer) * displayY, vec4uc(255, 255, 255, 255));
	for (int dX = 0; dX < displayX; dX++)
	{
		for (int dY = 0; dY < displayY; dY++)
		{
			const Bitmap bmp = fullLayers.compositeImage(finalSamples[resultIndex].lightColors);
			cout << finalSamples[resultIndex].baseFitness << endl;
			resultIndex++;

			bmpOut.copyIntoImage(bmp, (fullLayers.dimX + buffer) * dX, (fullLayers.dimY + buffer) * dY);
		}
	}
	LodePNG::save(bmpOut, params().debugDir + "finalResults.png");
}

void LightingExplorer::populateCandidatesExclusion(const LightingConstraints &constraints, const vector<float> &startX, const vector<LightingSample> &excludedSamples)
{
	GradientFreeProblem problem;
	problem.fitness = FitnessFunction([&](const vector<float> &x) {
		const Bitmap bmp = smallLayers.compositeImage(LightUtil::rawToLights(x));
		
		const float baseFitness = constraints.evalFitness(smallLayers, x);

		float exclusionFitness = 0.0f;
		const vector<float> signature = makeSignature(bmp);
		for (auto &e : excludedSamples)
		{
			const float dist = math::distL2(signature, e.signature) * Constants::signatureDistScale / signatureDim;
			//cout << dist << endl;
			if (dist < Constants::fitnessExclusionRadius)
			{
				exclusionFitness -= Constants::exclusionStrength * (1.0f - dist / Constants::fitnessExclusionRadius);
			}
		}
		//if(excludedSamples.size())
		//	cout << baseFitness << "," << exclusionFitness << endl;
		return baseFitness + exclusionFitness;
	});
	problem.render = RenderFunction([&](const vector<float> &x) {
		return fullLayers.compositeImage(LightUtil::rawToLights(x));
	});

	//GradientFreeOptRandomWalk opt;
	GradientFreeOptCMAES opt;
	vector< vector<float> > candidates;
	vector<float> bestResult = opt.optimize(problem, startX, candidates);

	for (auto &c : candidates)
	{
		LightingSample sample;
		sample.lightColors = LightUtil::rawToLights(c);
		sample.signature = makeSignature(sample.lightColors);
		sample.baseFitness = constraints.evalFitness(smallLayers, c);
		candidateSamples.push_back(sample);
	}
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
	const Bitmap bmp = smallLayers.compositeImage(lights);
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
