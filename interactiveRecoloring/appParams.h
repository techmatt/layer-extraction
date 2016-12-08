
struct Constants
{
	static const int a = 160;
};

struct AppParameters
{
	AppParameters()
	{
		ParameterFile file("recoloringParams.txt");

		//
		// directories
		//
		file.readParameter("debugDir", debugDir);
		file.readParameter("inputImage", inputImage);
		file.readParameter("editImage", editImage);

		//
		// superpixel constants
		//
		file.readParameter("superpixelIterations", superpixelIterations);
		file.readParameter("superpixelCount", superpixelCount);

		file.readParameter("spatialToColorScale", spatialToColorScale);
		file.readParameter("pixelNeighborCount", pixelNeighborCount);
		file.readParameter("superpixelNeighborCount", superpixelNeighborCount);
		file.readParameter("periodicBasisCount", periodicBasisCount);

		file.readParameter("stasisWeight", stasisWeight);
		file.readParameter("editWeight", editWeight);
		file.readParameter("regularizationWeight", regularizationWeight);

		file.readParameter("neighborhoodRegularizationWeight", neighborhoodRegularizationWeight);
	}

	string debugDir;

	string inputImage, editImage;

	int superpixelIterations;
	int superpixelCount;

	float spatialToColorScale;
	int pixelNeighborCount;
	int superpixelNeighborCount;
	int periodicBasisCount;

	float stasisWeight;
	float editWeight;
	float regularizationWeight;

	double neighborhoodRegularizationWeight;
};

extern AppParameters* g_appParams;
inline const AppParameters& appParams()
{
	return *g_appParams;
}

inline AppParameters& appParamsMutable()
{
	return *g_appParams;
}
