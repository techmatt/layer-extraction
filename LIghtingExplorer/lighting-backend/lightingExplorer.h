
struct LightingSample
{
	vector<vec3f> lightColors;
	vector<float> signature;
};

struct LightingExplorer
{
	void init();

	void populateCandidates(const LightingConstraints &constraints);

	//void maybeAddSample(const Bitmap &bmp, const vector<vec3f> &lightColors);

	vector<float> makeSignature(const vector<vec3f> &lights) const;
	vector<float> makeSignature(const Bitmap &bmp) const;

	int blocksX, blocksY, signatureDim;
	ImageLayers layers;
	//LSHEuclidean<size_t> hash;
	vector<LightingSample> candidateSamples;

	static const int signatureBlockSize = 8;
};
