
struct LightingSample
{
	vector<vec3f> lightColors;
	vector<float> signature;
	double baseFitness;
};

struct LightingExplorer
{
	void init();

	void populateCandidates(const LightingConstraints &constraints);

	void populateCandidatesExclusion(const LightingConstraints &constraints, const vector<float> &startX, const vector<LightingSample> &excludedSamples);

	//void maybeAddSample(const Bitmap &bmp, const vector<vec3f> &lightColors);

	vector<float> makeSignature(const vector<vec3f> &lights) const;
	vector<float> makeSignature(const Bitmap &bmp) const;

	int blocksX, blocksY, signatureDim;
	
	ImageLayers fullLayers;
	ImageLayers smallLayers;
	//LSHEuclidean<size_t> hash;
	vector<LightingSample> candidateSamples;
	vector<LightingSample> acceptedSamples;
};
