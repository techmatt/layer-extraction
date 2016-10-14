
struct Layer
{
	Grid2f g;
	vec3f baseColor;
};

struct ImageLayers
{
	void load(const string &baseDir);
	
	Bitmap compositeImage(const vector<vec3f> &layerColors) const;

	int dimX;
	int dimY;
	vector<Layer> layers;
};