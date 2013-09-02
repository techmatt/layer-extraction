
struct Layer
{
    Vec3f color;
    Vector<double> superpixelWeights;
};

struct SuperpixelLayerConstraint
{
    SuperpixelLayerConstraint()
    {

    }
    SuperpixelLayerConstraint(UINT _index, UINT _layerIndex, double _target, double _weight)
    {
        index = _index;
        layerIndex = _layerIndex;
        target = _target;
        weight = _weight;
    }

    UINT index;
    UINT layerIndex;
    double target;
    double weight;
};

struct LayerSet
{
    void Dump(const String &filename, const Vector<ColorCoordinate> &superpixelColors) const;
    Vector<Layer> layers;
    Vector<SuperpixelLayerConstraint> constraints;
};

class LayerExtractor
{
public:
    void Init(const AppParameters &parameters, const Bitmap &bmp);
    void InitLayersFromPixelConstraints(const AppParameters &parameters, const Bitmap &bmp, const Vector<PixelConstraint> &targetPixelColors, LayerSet &result);
    void InitLayersFromPalette(const AppParameters &parameters, const Bitmap &bmp, const Vector<Vec3f> &palette, LayerSet &result);
    void AddNegativeConstraints(const AppParameters &parameters, const Bitmap &bmp, LayerSet &result);
    void ExtractLayers(const AppParameters &parameters, const Bitmap &bmp, LayerSet &layers);

    void TestLayerRecoloring(const Bitmap &bmp, const LayerSet &layers) const;
    Bitmap RecolorSuperpixels(const Bitmap &bmp, const Vector<Vec3f> &newSuperpixelColors) const;
    Bitmap RecolorLayers(const Bitmap &bmp, const LayerSet &layers, const Vector<RGBColor> &newLayerColors) const;
    
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
    void TestNeighborWeights(const AppParameters &parameters, const Bitmap &bmp) const;

    Vector<ColorCoordinate> superpixelColors;
    Vector<SuperpixelNeighborhood> superpixelNeighbors;
    
    Grid<PixelNeighborhood> pixelNeighborhoods;

    SparseMatrix<double> WBase;
    int pass;
};
