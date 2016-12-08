
struct Superpixel
{
	Superpixel()
	{
		targetColorWeight = 0.0f;
		targetColor = vec3f::origin;
	}

	void addColorTarget(const vec3f &c, float weight)
	{
		targetColorWeight += weight;
		targetColor += c * weight;
	}

	SuperpixelCoord coord;

	//
	// constraint data
	//
	float targetColorWeight;
	vec3f targetColor;

	//
	// neighboring superpixel data
	//
	vector<unsigned int> neighborIndices;
	vector<double> neighborEmbeddingWeights;
	vector<double> neighborSimilarityWeights;

	//double shortestDist;
	//bool visited;

};

struct PixelNeighborhood
{
	// indices into the superpixel list
	vector<unsigned int> neighbors;

	vector<double> weights;
};

struct ImageSuperpixels
{
	void loadCoords(const vector<SuperpixelCoord> &coords)
	{
		superpixels.resize(coords.size());
		for (auto &i : iterate(superpixels))
			i.value.coord = coords[i.index];
	}

	void loadEdits(const Bitmap &imgInput, const Bitmap &imgEdit);

	void computeNeighborhoods(const Bitmap &imgInput);
	void computeNeighborhoodWeights(const Bitmap &imgInput);

	vector<double> computeWeights(const vector<unsigned int>& indices, const float * pixelFeatures);
	
	vector<Superpixel> superpixels;
	Grid2<int> assignments;

	Grid2<PixelNeighborhood> pixelNeighborhoods;

};
