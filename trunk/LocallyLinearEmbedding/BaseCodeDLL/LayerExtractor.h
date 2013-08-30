
struct Layer
{
    Vec3f color;

    //
    // superpixelSeeds is just used for debugging and visualization. Full constraint list is in LayerSet::constraints
    //
    Vector<UINT> superpixelSeeds;

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
    Vector<Layer> layers;
    Vector<SuperpixelLayerConstraint> constraints;
    Vector<SuperpixelLayerConstraint> baseConstraints;
};

class LayerExtractor
{
public:
    void Init(const AppParameters &parameters, const Bitmap &bmp);
    void InitLayers(const AppParameters &parameters, const Bitmap &bmp, const Vector<PixelConstraint> &targetPixelColors, LayerSet &result);
    void AddNegativeConstraints(const AppParameters &parameters, const Bitmap &bmp, LayerSet &result);
    void ExtractLayers(const AppParameters &parameters, const Bitmap &bmp, LayerSet &layers);
    
private:
    void ComputeSuperpixels(const AppParameters &parameters, const Bitmap &bmp);
    void ComputeNearestNeighbors(const AppParameters &parameters, const Bitmap &bmp);
    void ComputeNeighborWeights(const AppParameters &parameters, const Bitmap &bmp);
    void ComputeSourceDistance(const AppParameters &parameters, const Bitmap &bmp, UINT sourceSuperpixelIndex);
    void ComputeWeightMatrix(const AppParameters &parameters);
    
    Vector<double> ComputeWeights(const AppParameters &parameters, const Vector<UINT> &indices, const float *pixelFeatures);

    void VisualizeReconstruction(const AppParameters &parameters, const Bitmap &bmp, const LayerSet &layers) const;
    void VisualizeLayers(const AppParameters &parameters, const Bitmap &bmp, const LayerSet &layers) const;
    void VisualizeEmptyLayers(const AppParameters &parameters, const Bitmap &bmp, const LayerSet &layers) const;
    void VisualizeSuperpixels(const AppParameters &parameters, const Bitmap &bmp, const Vector<Vec3f> *newSuperpixelColors, const String &filename) const;
    void VisualizeNearestNeighbors(const AppParameters &parameters, const Bitmap &bmp) const;
    void TestNeighborWeights(const AppParameters &parameters, const Bitmap &bmp) const;

    Bitmap Recolor(const Bitmap &bmp, const Vector<Vec3f> &newSuperpixelColors) const;

    Vector<ColorCoordinate> superpixelColors;
    Vector<SuperpixelNeighborhood> superpixelNeighbors;
    
    Grid<PixelNeighborhood> pixelNeighborhoods;

    SparseMatrix<double> WBase;
    int pass;
};
