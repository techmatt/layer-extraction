
#include "main.h"

void ImageSuperpixels::loadEdits(const Bitmap &imgInput, const Bitmap & imgEdit)
{
	for (auto &p : imgEdit)
	{
		const vec4uc inputColor = imgInput(p.x, p.y);
		//cout << superpixels.size() << "," << assignments(p.x, p.y) << endl;
		Superpixel &super = superpixels[assignments(p.x, p.y)];

		if (p.value.getVec3() == vec3uc(255, 0, 255))
		{
			//super.addColorTarget(colorUtil::toVec3f(inputColor), appParams().stasisWeight);
			super.recordConstraint(Superpixel::ConstraintType::Stasis, colorUtil::toVec3f(inputColor));
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

	/*for (auto &s : superpixels)
	{
		if (s.targetColorWeight == 0.0)
			s.targetColor = vec3f::origin;
		else
			s.targetColor /= s.targetColorWeight;
	}*/
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