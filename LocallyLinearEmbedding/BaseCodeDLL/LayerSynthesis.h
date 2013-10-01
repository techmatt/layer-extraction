class LayerSynthesis
{
public:

	void Init(const GaussianPyramid &layers, const NeighborhoodGenerator &generator, UINT reducedDimension);

	PixelLayerSet Synthesize(const AppParameters &parameters, const GaussianPyramid &reference, const GaussianPyramid &original, const Vector<Vec2i> &pixels, const Grid<double> &updateSchedule, NeighborhoodGenerator &generator);

	//later..
	Vector<PixelLayer> SuggestLayers(const GaussianPyramid &original, const PixelLayer &mask);

private:
    void InitPCA(const GaussianPyramid &layers, const NeighborhoodGenerator &generator);
	void InitKDTree(const GaussianPyramid &layers, const NeighborhoodGenerator &generator, UINT reducedDimension);

	void SynthesizeStepInPlace(const AppParameters &parameters, const GaussianPyramid &reference, GaussianPyramid &target, const Vector<Vec2i> &pixels, const Vector<double> &updateSchedule, NeighborhoodGenerator &generator, Grid<Vec2i> &sourceCoordinates);
	void VisualizeLayers(const GaussianPyramid &layers, Bitmap &result);
	void VisualizeMatches(const AppParameters &parameters, const GaussianPyramid &reference, const GaussianPyramid &target, Vec2i targetPt, NeighborhoodGenerator &generator, String &filename, const Grid<Vec2i> &sourceCoordinates);
	void VisualizeNeighbors(const GaussianPyramid &reference, const GaussianPyramid &target, Vec2i targetPt, NeighborhoodGenerator &generator, String &filename);

	Vector<Vec2i> GetCandidateSourceNeighbors(Vec2i targetPt, const Grid<Vec2i> &sourceCoordinates, const GaussianPyramid &reference);
	double BestCoherentMatch(Vec2i targetPt, const GaussianPyramid &reference, const GaussianPyramid &target, NeighborhoodGenerator &generator, const Grid<Vec2i> &sourceCoordinates, Vec2i &outPt);
	double BestApproximateMatch(Vec2i targetPt, const GaussianPyramid &reference, const GaussianPyramid &target, NeighborhoodGenerator &generator, Vec2i &outPt);
	double NeighborhoodDistance(double* neighborhoodA, double* neighborhoodB, UINT dimension);
	double BestNeighborVotedMatch(Vec2i targetPt, const GaussianPyramid &reference, const GaussianPyramid &target, NeighborhoodGenerator &generator, Vec2i &outPt, int range);
	Vec2i BestMatch(const AppParameters &parameters, Vec2i targetPt, const GaussianPyramid &reference, const GaussianPyramid &target, NeighborhoodGenerator &generator, const Grid<Vec2i> &sourceCoordinates);


	UINT _reducedDimension;
	PCA<double> _pca;
	KDTree _tree;
	Vector<Vec2i> _treeCoordinates;

};


