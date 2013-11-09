
class LayerExtractorVideo
{
public:
    void Init(const AppParameters &parameters, const Video &video);
    void InitLayersFromPalette(const AppParameters &parameters, const Video &video, const Vector<Vec3f> &palette, LayerSet &result);
	void AddNegativeConstraints(const AppParameters &parameters, const Video &video, LayerSet &result);
    void ExtractLayers(const AppParameters &parameters, const Video &video, LayerSet &layers);
	
    Bitmap RecolorSuperpixels(const Video &video, UINT frameIndex, const Vector<Vec3f> &newSuperpixelColors) const;
    Bitmap RecolorLayers(const Video &video, UINT frameIndex, const LayerSet &layers, const Vector<RGBColor> &newLayerColors) const;

    Bitmap VisualizeLayer(const AppParameters &parameters, const Video &video, UINT frameIndex, UINT layerIndex, const LayerSet &layers) const;

	Vector<PixelLayer> GetPixelLayers(const Video &video, UINT frameIndex, const LayerSet &layers) const;

	const Vector<ColorCoordinateVideo>& SuperpixelColors() const
    {
        return superpixelColors;
    }

private:
    void ComputeSuperpixels(const AppParameters &parameters, const Video &video);
    void ComputeNearestNeighbors(const AppParameters &parameters, const Video &video);
    void ComputeNeighborWeights(const AppParameters &parameters, const Video &video);
    void ComputeWeightMatrix(const AppParameters &parameters);
    
    Vector<double> ComputeWeights(const AppParameters &parameters, const Vector<UINT> &indices, const float *pixelFeatures);

    void VisualizeReconstruction(const AppParameters &parameters, const Video &video, UINT frameIndex, const LayerSet &layers) const;
    void VisualizeLayers(const AppParameters &parameters, const Video &video, UINT frameIndex, const LayerSet &layers) const;
    void VisualizeLayerConstraints(const AppParameters &parameters, const Video &video, UINT frameIndex, const LayerSet &layers) const;
    void VisualizeSuperpixels(const AppParameters &parameters, const Video &video, UINT frameIndex, const Vector<Vec3f> *newSuperpixelColors, const String &filename) const;
    void VisualizeNearestNeighbors(const AppParameters &parameters, const Video &video, UINT frameIndex) const;
	
    void TestNeighborWeights(const AppParameters &parameters, const Video &video) const;

    Vector<ColorCoordinateVideo> superpixelColors;
    Vector<SuperpixelNeighborhood> superpixelNeighbors;
    
    Vector< Grid<PixelNeighborhood> > pixelNeighborhoods;

    SparseMatrix<double> WBase;
    int pass;
};
