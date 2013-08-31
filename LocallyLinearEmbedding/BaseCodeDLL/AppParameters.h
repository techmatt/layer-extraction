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

        layerConstraintWeight = file.GetDouble("layerConstraintWeight");
        sumToOneWeight = file.GetDouble("sumToOneWeight");
        reconstructionWeight = file.GetDouble("reconstructionWeight");
        regularizationWeight = file.GetDouble("regularizationWeight");
        negativeSupressionWeight = file.GetDouble("negativeSupressionWeight");
    }

    String imageFile;
    String maskFile;

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

    double layerConstraintWeight;
    double sumToOneWeight;
    double reconstructionWeight;
    double regularizationWeight;
    double negativeSupressionWeight;
};