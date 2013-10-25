
class LayerTextureSynthesizer
{
public:
	void Synthesize(const PixelLayerSet &layers, const PixelLayerSet &rgb, const AppParameters &parameters,
		NeighborhoodGenerator &generator, Vector<int> order);

private:
	void Synthesize(const Vector<PixelLayerSet> &layerfeatures, const AppParameters &parameters,
		NeighborhoodGenerator &generator, Vector<int> order, int iteration,
		PixelLayerSet &synthesized, Vector< Grid<Vec2i> > &coordinateset);

	Vec2i BestMatch(const Vector<PixelLayerSet> &layerfeatures, PCA<double> &pca, KDTree &kdtree, const Vector<Vec2i> &treeCoordinates,
		NeighborhoodGenerator &generator, const PixelLayerSet &synthesized, const Vector< Grid<Vec2i> > &coordinateset, Vector<int> order, int iteration,
		int x, int y, double *neighbourhood, double *transformedNeighbourhood, int width, int height,
		double *coherentneighbourhood, double *transformedCohNeighbourhood);
	double BestApproximateMatch(KDTree &kdtree, const Vector<Vec2i> &treeCoordinates, 
						        Vec2i &outPt, double *transformedNeighbourhood);
	double BestCoherentMatch(const Vector<PixelLayerSet> &layerfeatures, PCA<double> &pca, NeighborhoodGenerator &generator, const Vector< Grid<Vec2i> > &coordinateset, 
							 Vector<int> order, int iteration, int width, int height, int x, int y, Vec2i &outPt, double *transformedNeighbourhood,
							 double *coherentneighbourhood, double *transformedCohNeighbourhood);

	double NeighborhoodDistance(double* neighborhoodA, double* neighborhoodB, UINT dimension);

	void ComputePCA(const AppParameters &parameters, const Vector<PixelLayerSet> &layerfeatures, const NeighborhoodGenerator &generator, Vector<int> order, int iteration, PCA<double> &pca);
	void ConstructKDTree(PCA<double> &pca, const Vector<PixelLayerSet> &layerfeatures, const NeighborhoodGenerator &generator, Vector<int> order, int iteration, Vector<Vec2i> &treeCoordinates, KDTree& kdtree);

	void VisualizeMatches(const PixelLayerSet &layers, const PixelLayerSet &synthlayers, Vector<int> order, int iteration, int nradius,
						  const Vec2i &pt, const Vec2i &matchpt) const; // obsolete
	void Write(const Vector<PixelLayerSet> &layerfeatures, PixelLayerSet &synth, const Vector< Grid<Vec2i> > &coordinateset, int width, int height, int pad, String cond) const;

	UINT _reducedDimension;
	double _kappa;
	int _dimension;

	String _outputdir;
	int _coherentcount;
};

