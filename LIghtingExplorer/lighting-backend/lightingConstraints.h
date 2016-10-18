
struct PixelConstraint
{
	float weight;
	vec3f targetColor;
};

struct LightingConstraints
{
	//function<float(const vector<float> &)> makeFitnessFunction() const;

	void init(const Bitmap &startImage, float startWeight, const Bitmap &targetImage, float targetWeight);
	double evalFitness(const Bitmap &bmp, const vector<float> &x) const;
	double evalFitness(const ImageLayers &l, const vector<float> &x) const;

	void saveDebug();

	Grid2<PixelConstraint> pixelConstraints;
};
