
class LayerTextureSynthesizer
{
public:
	void Synthesize(const PixelLayerSet &layers, const PixelLayerSet &rgb, const AppParameters &parameters,
		NeighborhoodGenerator &generator, double kappa, Vector<int> order);

private:
	void Synthesize(const PixelLayerSet &layers, const PixelLayerSet &rgb, const AppParameters &parameters,
		NeighborhoodGenerator &generator, double kappa, Vector<int> order, int iteration,
		PixelLayerSet &synthesized, Vector< Grid<Vec2i> > &coordinateset);

	Vec2i BestMatch(const PixelLayerSet &layers, PCA<double> &pca, KDTree &kdtree, const Vector<Vec2i> &treeCoordinates,
		NeighborhoodGenerator &generator, const Grid<Vec2i> &coordinates, Vector<int> order, int iteration,
		int x, int y, double kappa, double *neighbourhood, double *transformedNeighbourhood, int width, int height,
		double *coherentneighbourhood, double *transformedCohNeighbourhood);

	PCA<double> ComputePCA(const AppParameters &parameters, const PixelLayerSet &layers, const NeighborhoodGenerator &generator, Vector<int> order, int iteration);
	KDTree ConstructKDTree(PCA<double> &pca, const PixelLayerSet &layers, const NeighborhoodGenerator &generator, Vector<int> order, int iteration, Vector<Vec2i> &treeCoordinates);

	void Write(const PixelLayerSet &layers, PixelLayerSet &synth, const Vector< Grid<Vec2i> > &coordinateset, int width, int height, int pad, String cond);

	UINT _reducedDimension;

	String _outputdir;
	int _coherentcount;
};

