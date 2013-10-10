
class TextureSynthesis
{
public:

	void Init(const AppParameters &parameters, const GaussianPyramid &exemplar, const NeighborhoodGenerator &generator, int nlevels, int reducedDimension);

	void Synthesize(const GaussianPyramid &rgbpyr, const GaussianPyramid &exemplar, const AppParameters &parameters, NeighborhoodGenerator &generator, double kappa);

private:
    void InitPCA(const AppParameters &parameters, const GaussianPyramid &exemplar, const NeighborhoodGenerator &generator);
	void InitKDTree(const GaussianPyramid &exemplar, const NeighborhoodGenerator &generator, UINT reducedDimension);

	Vec2i BestMatch(const GaussianPyramid &exemplar, NeighborhoodGenerator &generator, const Grid<Vec2i> &coordinates,
				    int level, int x, int y, double coherence,
					double *neighbourhood, double *transformedNeighbourhood, int width, int height,
					double *coherentneighbourhood, double *transformedCohNeighbourhood);
	double BestApproximateMatch(const GaussianPyramid &exemplar, NeighborhoodGenerator &generator, int level,
								Vec2i &outPt, double *transformedNeighbourhood);
	double BestCoherentMatch(const GaussianPyramid &exemplar, NeighborhoodGenerator &generator, const Grid<Vec2i> &coordinates,
							 int level, int width, int height, int x, int y, Vec2i &outPt, double *transformedNeighbourhood,
							 double *coherentneighbourhood, double *transformedCohNeighbourhood);

	double NeighborhoodDistance(double* neighborhoodA, double* neighborhoodB, UINT dimension);

	void WriteImage(const GaussianPyramid &rgbpyr, const GaussianPyramid &exemplar, const Grid<Vec2i> &coordinates, int level, int width, int height, int pad, String label);


	UINT _reducedDimension;
	Vector< PCA<double> > _pca;
	Vector<KDTree> _tree;
	Vector< Vector<Vec2i> > _treeCoordinates;

	bool _usepca;
	int _coherentcount;
	
	bool _debug;
	String _debugoutdir;
};