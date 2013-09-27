struct AppParameters
{
    void Init(const String &parameterFilename)
    {
        ParameterFile file(parameterFilename);

        imageFile = file.GetRequiredString("imageFile");
        maskFile = file.GetRequiredString("maskFile");

        periodicBasisCount = file.GetInteger("periodicBasisCount");
        superpixelIterations = file.GetInteger("superpixelIterations");

        spatialToColorScale = file.GetFloat("spatialToColorScale");

        pixelNeighborCount = file.GetInteger("pixelNeighborCount");
        superpixelNeighborCount = file.GetInteger("superpixelNeighborCount");

        superpixelCount = file.GetInteger("superpixelCount");

        weightRegularizationTerm = file.GetDouble("weightRegularizationTerm");

        userConstraintWeight = file.GetDouble("userConstraintWeight");
        distantConstraintWeight = file.GetDouble("distantConstraintWeight");
        colorInertiaWeight = file.GetDouble("colorInertiaWeight");

        pixelConstraintWeight = file.GetDouble("pixelConstraintWeight");
        sumToOneWeight = file.GetDouble("sumToOneWeight");
        reconstructionWeight = file.GetDouble("reconstructionWeight");
        regularizationWeight = file.GetDouble("regularizationWeight");
        negativeSupressionWeight = file.GetDouble("negativeSupressionWeight");
		preferenceWeight = file.GetDouble("preferenceWeight");


        useKMeansPalette = file.GetBoolean("useKMeansPalette");
        KMeansPaletteSize = file.GetInteger("KMeansPaletteSize");

		allTargetLayers = file.GetString("allTargetLayers","").FindAndReplace(" ","").Partition(",");
		targetLayers = file.GetString("targetLayers","").FindAndReplace(" ","").Partition(",");
		refLayers = file.GetString("refLayers","").FindAndReplace(" ","").Partition(",");
		Vector<String> temp = file.GetString("updateSchedule","").FindAndReplace(" ","").Partition(",");
		for (UINT i=0; i<temp.Length(); i++)
			updateSchedule.PushEnd(temp[i].ConvertToDouble());

		targetImageFile = file.GetRequiredString("targetImageFile");
        targetMaskFile = file.GetRequiredString("targetMaskFile");

		neighborhoodSize = file.GetDouble("neighborhoodSize");
		reducedDimension = file.GetInteger("reducedDimension");

		coherenceParameter = file.GetDouble("coherenceParameter");
		useMajorityVote = file.GetBoolean("useMajorityVote");
		neighborRange = file.GetInteger("neighborRange");


    }

    String imageFile;
    String maskFile;

    bool useKMeansPalette;
    int KMeansPaletteSize;

    int periodicBasisCount;
    int superpixelIterations;

    float spatialToColorScale;

    int pixelNeighborCount;
    int superpixelNeighborCount;
    
    int superpixelCount;

    double weightRegularizationTerm;
    double userConstraintWeight;
    double distantConstraintWeight;
    double colorInertiaWeight;

    double pixelConstraintWeight;
    double sumToOneWeight;
    double reconstructionWeight;
    double regularizationWeight;
    double negativeSupressionWeight;
	double preferenceWeight;	
	Vector<String> allTargetLayers;
	Vector<String> targetLayers;
	Vector<String> refLayers;
	String mask;
	Vector<double> updateSchedule;

	String targetImageFile;
	String targetMaskFile;

	double neighborhoodSize;
	int reducedDimension;

	double coherenceParameter;
	bool useMajorityVote;
	int neighborRange;
};