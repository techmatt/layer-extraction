
class LayerExtractor
{
public:
    void Init(const AppParameters &parameters, const Bitmap &bmp);
    void InitLayersFromPixelConstraints(const AppParameters &parameters, const Bitmap &bmp, const Vector<PixelConstraint> &targetPixelColors, LayerSet &result);
    void InitLayersFromPalette(const AppParameters &parameters, const Bitmap &bmp, const Vector<Vec3f> &palette, LayerSet &result);
	void AddLayerPreferenceConstraints(const AppParameters &parameters, const Bitmap &bmp,LayerSet &result);
    void AddNegativeConstraints(const AppParameters &parameters, const Bitmap &bmp, LayerSet &result);
    void ExtractLayers(const AppParameters &parameters, const Bitmap &bmp, LayerSet &layers);
	bool CorrectLayerSet(const AppParameters &parameters, const Bitmap &bmp, LayerSet &layers);
	void AddMidpointConstraints(const AppParameters &parameters, const Bitmap &bmp, LayerSet &result);

	Vector<PixelConstraint> ComputePalette(const String &filename, const Bitmap &bmp) const;

    void TestLayerRecoloring(const Bitmap &bmp, const LayerSet &layers) const;
    Bitmap RecolorSuperpixels(const Bitmap &bmp, const Vector<Vec3f> &newSuperpixelColors) const;
    Bitmap RecolorLayers(const Bitmap &bmp, const LayerSet &layers, const Vector<RGBColor> &newLayerColors) const;

	Vector<PixelLayer> GetPixelLayers(const Bitmap &bmp, const LayerSet &layers) const;
    
    const Vector<ColorCoordinate>& SuperpixelColors() const
    {
        return superpixelColors;
    }
private:
    void ComputeSuperpixels(const AppParameters &parameters, const Bitmap &bmp);
    void ComputeNearestNeighbors(const AppParameters &parameters, const Bitmap &bmp);
    void ComputeNeighborWeights(const AppParameters &parameters, const Bitmap &bmp);
    void ComputeSourceDistance(const AppParameters &parameters, const Bitmap &bmp, UINT sourceSuperpixelIndex);
    void ComputeWeightMatrix(const AppParameters &parameters);
    
    Vector<double> ComputeWeights(const AppParameters &parameters, const Vector<UINT> &indices, const float *pixelFeatures);

    void VisualizeReconstruction(const AppParameters &parameters, const Bitmap &bmp, const LayerSet &layers) const;
    void VisualizeLayers(const AppParameters &parameters, const Bitmap &bmp, const LayerSet &layers) const;
    void VisualizeLayerConstraints(const AppParameters &parameters, const Bitmap &bmp, const LayerSet &layers) const;
    void VisualizeSuperpixels(const AppParameters &parameters, const Bitmap &bmp, const Vector<Vec3f> *newSuperpixelColors, const String &filename) const;
    void VisualizeNearestNeighbors(const AppParameters &parameters, const Bitmap &bmp) const;
	void VisualizeLayerPalette(const AppParameters &parameters, const Bitmap &bmp, const LayerSet &layers, int index, int superpixelIndex) const;
	void VisualizeLayerPreferences(const AppParameters &parameters, const Bitmap &bmp, const LayerSet &layers) const;
    void VisualizeLayerGrid(const AppParameters &parameters, const Bitmap &bmp, const LayerSet &layers) const;

    void TestNeighborWeights(const AppParameters &parameters, const Bitmap &bmp) const;

    Vector<ColorCoordinate> superpixelColors;
    Vector<SuperpixelNeighborhood> superpixelNeighbors;
    
    Grid<PixelNeighborhood> pixelNeighborhoods;

    SparseMatrix<double> WBase;
    int pass;
	int correctionPass;
};
