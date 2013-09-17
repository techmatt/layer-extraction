
class LayerSynthesis
{
public:

	void Init(const PixelLayerSet &layers, const NeighborhoodGenerator &generator, UINT reducedDimension);

	PixelLayerSet Synthesize(const AppParameters &parameters, const PixelLayerSet &original, const Vector<Vec2i> &pixels, const Grid<double> &updateSchedule);

	//later..
	Vector<PixelLayer> SuggestLayers(const PixelLayerSet &original, const PixelLayer &mask);

private:
    void InitPCA(const PixelLayerSet &layers, const NeighborhoodGenerator &generator);
	void InitKDTree(const PixelLayerSet &layers, const NeighborhoodGenerator &generator, UINT reducedDimension);

	UINT _reducedDimension;
	PCA<double> _pca;
	KDTree _tree;
	Vector<Vec2i> _treeCoordinates;

};

