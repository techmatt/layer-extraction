
#include "main.h"

void ImageSuperpixels::loadEdits(const Bitmap &imgInput, const Bitmap & imgEdit)
{
	for (auto &s : superpixels)
	{
		s.targetColor = vec3f::origin;
		s.targetColorWeight = appParams().regularizationWeight;
		s.constraintType = Superpixel::ConstraintType::Regularization;
	}

	for (auto &p : imgEdit)
	{
		const vec4uc inputColor = imgInput(p.x, p.y);
		//cout << superpixels.size() << "," << assignments(p.x, p.y) << endl;
		Superpixel &super = superpixels[assignments(p.x, p.y)];

		if (p.value.getVec3() == vec3uc(255, 0, 255))
		{
			//super.addColorTarget(colorUtil::toVec3f(inputColor), appParams().stasisWeight);
			super.recordConstraint(Superpixel::ConstraintType::ActiveStasis, super.coord.color);
		}
		else if (p.value.getVec3() != inputColor.getVec3())
		{
			//super.addColorTarget(colorUtil::toVec3f(p.value), appParams().editWeight);
			super.recordConstraint(Superpixel::ConstraintType::Edit, colorUtil::toVec3f(p.value));
		}
		else
		{
			//super.addColorTarget(colorUtil::toVec3f(inputColor), appParams().regularizationWeight);
		}
	}

	if (appParams().equalizeCostraintLightness)
	{
		for (auto &s : superpixels)
		{
			if (s.constraintType == Superpixel::ConstraintType::Edit)
			{
				const float targetLightness = s.coord.color.length();
				const float constraintLightness = s.targetColor.length();
				const float scale = targetLightness / constraintLightness;
				s.targetColor *= scale;
				s.targetColor.x = math::clamp(s.targetColor.x, 0.0f, 1.0f);
				s.targetColor.y = math::clamp(s.targetColor.y, 0.0f, 1.0f);
				s.targetColor.z = math::clamp(s.targetColor.z, 0.0f, 1.0f);
			}
		}
	}

	/*for (auto &s : superpixels)
	{
		if (s.targetColorWeight == 0.0)
			s.targetColor = vec3f::origin;
		else
			s.targetColor /= s.targetColorWeight;
	}*/

	computeConstraintDists();

	vector<float> dists;
	for (auto &s : superpixels)
	{
		//cout << s.constraintDist << endl;
		dists.push_back(s.constraintDist);
	}
	sort(dists.begin(), dists.end());
	maxConstraintDist = dists.back();
	const float passiveThreshold = util::clampedRead(dists, math::round((1.0f - appParams().passiveStasisQuertile) * dists.size()));

	int editCount = 0;
	int activeStasisCount = 0;
	int passiveStasisCount = 0;
	for (auto &s : superpixels)
	{
		if (s.constraintDist >= passiveThreshold)
		{
			s.recordConstraint(Superpixel::ConstraintType::PassiveStasis, s.coord.color);
		}
		if (s.constraintType == Superpixel::ConstraintType::Edit) editCount++;
		if (s.constraintType == Superpixel::ConstraintType::ActiveStasis) activeStasisCount++;
		if (s.constraintType == Superpixel::ConstraintType::PassiveStasis) passiveStasisCount++;
	}
	cout << "edit superpixels: " << editCount << endl;
	cout << "active stasis superpixels: " << activeStasisCount << endl;
	cout << "passive stasis superpixels: " << passiveStasisCount << endl;
	cout << "total superpixels: " << superpixels.size() << endl;
}

void ImageSuperpixels::computeNeighborhoods(const Bitmap &imgInput)
{
	ComponentTimer timer("Computing nearest neighbors");

	NearestNeighborSearchBruteForce<float> search;
	
	vector<const float*> superpixelFeatures;
	for (const Superpixel &s : superpixels)
	{
		superpixelFeatures.push_back(s.coord.features);
	}

	search.init(superpixelFeatures, 5, max(appParams().superpixelNeighborCount, appParams().pixelNeighborCount));

	pixelNeighborhoods.allocate(imgInput.getDimensions());
	for (auto &p : pixelNeighborhoods)
	{
		const SuperpixelCoord curCoord(imgInput, p.x, p.y);
		p.value.neighbors = search.kNearest(curCoord.features, appParams().pixelNeighborCount, 0.0f);
	}

	for (auto &s : iterate(superpixels))
	{
		s.value.neighborIndices = search.kNearest(s.value.coord.features, appParams().pixelNeighborCount, 0.0f);
		if (util::contains(s.value.neighborIndices, (unsigned int)s.index))
		{
			util::removeSwap(s.value.neighborIndices, util::findFirstIndex(s.value.neighborIndices, (unsigned int)s.index));
		}
		else
		{
			s.value.neighborIndices.pop_back();
		}
	}
}

void ImageSuperpixels::computeNeighborhoodWeights(const Bitmap &imgInput)
{
	ComponentTimer timer("Computing neighborhood weights");

	cout << "Computing pixel weights" << endl;
	for(auto &p : pixelNeighborhoods)
	{
		SuperpixelCoord curCoord(imgInput, p.x, p.y);
		p.value.weights = computeWeights(p.value.neighbors, curCoord.features);
	}

	cout << "Computing superpixel weights" << endl;
	for (auto &s : superpixels)
	{
		s.neighborEmbeddingWeights = computeWeights(s.neighborIndices, s.coord.features);

		s.neighborSimilarityWeights.resize(s.neighborEmbeddingWeights.size());
		for (auto &d : iterate(s.neighborSimilarityWeights))
		{
			d.value = vec3f::dist(s.coord.color, superpixels[s.neighborIndices[d.index]].coord.color);
		}
	}
}

vector<double> ImageSuperpixels::computeWeights(const vector<unsigned int> &indices, const float *pixelFeatures)
{
	//
	// Remember that for forming linear combinations, we consider only the color terms (3 dimensions) and not the spatial ones!
	//

	const size_t k = indices.size();
	DenseMatrix<double> z(k, 3);
	for (size_t neighborIndex = 0; neighborIndex < k; neighborIndex++)
	{
		const SuperpixelCoord &neighbor = superpixels[indices[neighborIndex]].coord;
		for (size_t dimensionIndex = 0; dimensionIndex < 3; dimensionIndex++)
		{
			z(neighborIndex, dimensionIndex) = neighbor.features[dimensionIndex] - pixelFeatures[dimensionIndex];
		}
	}

	DenseMatrix<double> G = z * z.getTranspose();

	//
	// Add weight regularization term
	//
	for (size_t neighborIndex = 0; neighborIndex < k; neighborIndex++)
	{
		G(neighborIndex, neighborIndex) += appParams().neighborhoodRegularizationWeight;
	}

	G.invertInPlace();

	vector<double> columnVector(k, 1.0);
	vector<double> result = DenseMatrix<double>::multiply(G, columnVector);

	double sum = 0.0;
	for (auto &v : result)
		sum += v;

	double scale = 1.0 / sum;
	for (auto &v : result)
		v *= scale;
	
	return result;
}

void ImageSuperpixels::computeConstraintDists()
{
	vector<const Superpixel*> constrainedSuperpixels;
	for (const Superpixel &s : superpixels)
	{
		if (s.constraintType == Superpixel::ConstraintType::Edit)
			constrainedSuperpixels.push_back(&s);
	}

	vector<float> colorDists;
	vector<float> spatialDists;
	for (Superpixel &s : superpixels)
	{
		float bestDist = numeric_limits<float>::max();
		for (const Superpixel *cPtr : constrainedSuperpixels)
		{
			const Superpixel &c = *cPtr;
			const float colorDist = vec3f::dist(s.coord.color, c.coord.color);
			const float spatialDist = vec2f::dist(vec2f(s.coord.coord), vec2f(c.coord.coord)) / assignments.getDimX();
			colorDists.push_back(colorDist);
			spatialDists.push_back(spatialDist);
		}
	}
	if (colorDists.size() == 0) return;

	sort(colorDists.begin(), colorDists.end());
	sort(spatialDists.begin(), spatialDists.end());

	const float colorDistScale = 1.0f / colorDists[math::round(colorDists.size() * 0.75f)];
	const float spatialDistScale = 1.0f / spatialDists[math::round(spatialDists.size() * 0.75f)];

	for (Superpixel &s : superpixels)
	{
		float bestDist = numeric_limits<float>::max();
		for (const Superpixel *cPtr : constrainedSuperpixels)
		{
			const Superpixel &c = *cPtr;
			const float colorDist = colorDistScale * vec3f::dist(s.coord.color, c.coord.color);
			const float spatialDist = spatialDistScale * vec2f::dist(vec2f(s.coord.coord), vec2f(c.coord.coord)) / assignments.getDimX();

			//const float curDist = min(colorDist, spatialDist);
			const float curDist = colorDist * 0.1f + spatialDist;
			bestDist = min(bestDist, curDist);
		}
		s.constraintDist = bestDist;
	}

	/*priority_queue<SuperpixelQueueEntry> queue;

	for (Superpixel &s : superpixels)
	{
		if (s.constraintType == Superpixel::ConstraintType::Edit)
		{
			queue.push(SuperpixelQueueEntry(s, 0.0));
			s.constraintDist = 0.0;
		}
		else
		{
			s.constraintDist = numeric_limits<float>::max();
		}
		s.visited = false;
	}

	float maxObservedDist = 0.0f;
	while (!queue.empty())
	{
		SuperpixelQueueEntry curEntry = queue.top();
		queue.pop();

		if (!curEntry.s->visited)
		{
			curEntry.s->visited = true;
			for (size_t neighborIndex = 0; neighborIndex < curEntry.s->neighborIndices.size(); neighborIndex++)
			{
				Superpixel &curNeighbor = superpixels[curEntry.s->neighborIndices[neighborIndex]];
				const float newDist = curEntry.s->constraintDist + (float)curEntry.s->neighborSimilarityWeights[neighborIndex];
				if (newDist < curNeighbor.constraintDist)
				{
					curNeighbor.constraintDist = newDist;
					maxObservedDist = max(maxObservedDist, newDist);
					queue.push(SuperpixelQueueEntry(curNeighbor, curNeighbor.constraintDist));
				}
			}
		}
	}

	for (Superpixel &s : superpixels)
	{
		if (s.constraintDist == numeric_limits<float>::max())
			s.constraintDist = maxObservedDist;
	}*/
}
