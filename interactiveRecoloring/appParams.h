
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
		file.readParameter("vizDir", vizDir);
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

		file.readParameter("passiveStasisWeight", passiveStasisWeight);
		file.readParameter("passiveStasisQuertile", passiveStasisQuertile);
		file.readParameter("activeStasisWeight", activeStasisWeight);
		file.readParameter("editWeight", editWeight);
		file.readParameter("regularizationWeight", regularizationWeight);

		file.readParameter("neighborhoodRegularizationWeight", neighborhoodRegularizationWeight);
	}

	string vizDir;

	string inputImage, editImage;

	int superpixelIterations;
	int superpixelCount;

	float spatialToColorScale;
	int pixelNeighborCount;
	int superpixelNeighborCount;
	int periodicBasisCount;

	float passiveStasisWeight;
	float activeStasisWeight;
	float editWeight;
	float regularizationWeight;
	float passiveStasisQuertile;

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
