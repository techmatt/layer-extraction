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

		manifoldWeight = file.GetDouble("manifoldWeight");
        pixelConstraintWeight = file.GetDouble("pixelConstraintWeight");
        sumToOneWeight = file.GetDouble("sumToOneWeight");
        reconstructionWeight = file.GetDouble("reconstructionWeight");
        regularizationWeight = file.GetDouble("regularizationWeight");
        negativeSupressionWeight = file.GetDouble("negativeSupressionWeight");
		preferenceWeight = file.GetDouble("preferenceWeight");
		midpointWeight = file.GetDouble("midpointWeight");


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

		neighborhoodSize = file.GetInteger("neighborhoodSize");
		reducedDimension = file.GetInteger("reducedDimension");

		coherenceParameter = file.GetDouble("coherenceParameter");
		useMajorityVote = file.GetBoolean("useMajorityVote");
		neighborRange = file.GetInteger("neighborRange");
		pyramidDepth = file.GetInteger("pyramidDepth");

		texsyn_exemplar = file.GetString("texsyn_exemplar", "");
		texsyn_outputwidth = file.GetInteger("texsyn_outputwidth");
		texsyn_outputheight = file.GetInteger("texsyn_outputheight");
		texsyn_nlevels = file.GetInteger("texsyn_nlevels");
		texsyn_neighbourhoodsize = file.GetInteger("texsyn_neighbourhoodsize");
		texsyn_kappa = file.GetDouble("texsyn_kappa");
		texsyn_usepca = file.GetBoolean("texsyn_usepca");
		texsyn_pcadim = file.GetInteger("texsyn_pcadim");
		texsyn_usergb = file.GetBoolean("texsyn_usergb");
		texsyn_uselayers = file.GetBoolean("texsyn_uselayers");
		texsyn_klayers = file.GetInteger("texsyn_klayers");
		texsyn_initrandsize = file.GetInteger("texsyn_initrandsize");

    }

    String imageFile;
    String maskFile;

    bool useKMeansPalette;
    int KMeansPaletteSize;

	bool useTextureFeatures;

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
	double midpointWeight;
	double manifoldWeight;

	Vector<String> allTargetLayers;
	Vector<String> targetLayers;
	Vector<String> refLayers;
	String mask;
	Vector<double> updateSchedule;

	String targetImageFile;
	String targetMaskFile;

	int neighborhoodSize;
	int reducedDimension;

	double coherenceParameter;
	bool useMajorityVote;
	int neighborRange;
	int pyramidDepth;

	String texsyn_exemplar;
	int texsyn_outputwidth;
	int texsyn_outputheight;
	int texsyn_nlevels;
	int texsyn_neighbourhoodsize;
	double texsyn_kappa;
	bool texsyn_usepca;
	int texsyn_pcadim;
	bool texsyn_usergb;
	bool texsyn_uselayers;
	int texsyn_klayers;
	int texsyn_initrandsize;
};