class LayerSynthesis
{
public:

	void Init(const PixelLayerSet &layers, const NeighborhoodGenerator &generator, UINT reducedDimension);

	PixelLayerSet Synthesize(const AppParameters &parameters, const PixelLayerSet &reference, const PixelLayerSet &original, const Vector<Vec2i> &pixels, const Grid<double> &updateSchedule, NeighborhoodGenerator &generator);

	//later..
	Vector<PixelLayer> SuggestLayers(const PixelLayerSet &original, const PixelLayer &mask);

private:
    void InitPCA(const PixelLayerSet &layers, const NeighborhoodGenerator &generator);
	void InitKDTree(const PixelLayerSet &layers, const NeighborhoodGenerator &generator, UINT reducedDimension);

	void SynthesizeStepInPlace(const AppParameters &parameters, const PixelLayerSet &reference, PixelLayerSet &target, const Vector<Vec2i> &pixels, const Vector<double> &updateSchedule, NeighborhoodGenerator &generator, Grid<Vec2i> &sourceCoordinates);
	void VisualizeLayers(const PixelLayerSet &layers, Bitmap &result);
	void VisualizeMatches(const AppParameters &parameters, const PixelLayerSet &reference, const PixelLayerSet &target, Vec2i targetPt, NeighborhoodGenerator &generator, String &filename, const Grid<Vec2i> &sourceCoordinates);
	void VisualizeNeighbors(const PixelLayerSet &reference, const PixelLayerSet &target, Vec2i targetPt, NeighborhoodGenerator &generator, String &filename);

	Vector<Vec2i> GetCandidateSourceNeighbors(Vec2i targetPt, const Grid<Vec2i> &sourceCoordinates, const PixelLayerSet &reference);
	double BestCoherentMatch(Vec2i targetPt, const PixelLayerSet &reference, const PixelLayerSet &target, NeighborhoodGenerator &generator, const Grid<Vec2i> &sourceCoordinates, Vec2i &outPt);
	double BestApproximateMatch(Vec2i targetPt, const PixelLayerSet &reference, const PixelLayerSet &target, NeighborhoodGenerator &generator, Vec2i &outPt);
	double NeighborhoodDistance(double* neighborhoodA, double* neighborhoodB, UINT dimension);


	UINT _reducedDimension;
	PCA<double> _pca;
	KDTree _tree;
	Vector<Vec2i> _treeCoordinates;

};

