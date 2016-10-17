
struct PixelConstraint
{
	float weight;
	vec3f targetColor;
};

struct LightingConstraints
{
	//function<float(const vector<float> &)> makeFitnessFunction() const;

	void init(const Bitmap &startImage, float startWeight, const Bitmap &targetImage, float targetWeight);
	float evalFitness(const ImageLayers &l, const vector<float> &x) const;

	Grid2<PixelConstraint> pixelConstraints;
};
