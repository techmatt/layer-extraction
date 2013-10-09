
class TextureSynthesis
{
public:

	void Init(const String& exemplarname, const GaussianPyramid &exemplar, const NeighborhoodGenerator &generator, int nlevels, int reducedDimension, int coherenceK, int coherenceNSize);

	void Synthesize(const GaussianPyramid &exemplar, int outputwidth, int outputheight, NeighborhoodGenerator &generator, double kappa);

private:
    void InitPCA(const String& exemplarname, const GaussianPyramid &exemplar, const NeighborhoodGenerator &generator);
	void InitKDTree(const GaussianPyramid &exemplar, const NeighborhoodGenerator &generator, UINT reducedDimension);
	void InitCoherence(const GaussianPyramid &exemplar, int coherenceK, int coherenceNSize);

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

	void WriteImage(const GaussianPyramid &exemplar, const Grid<Vec2i> &coordinates, int level, int width, int height, int pad, String label);


	UINT _reducedDimension;
	Vector< PCA<double> > _pca;
	Vector<KDTree> _tree;
	Vector< Vector<Vec2i> > _treeCoordinates;
	Grid< Vector<Vec2i> > _coherenceCandidates;

	int _coherentcount;
	
	bool _debug;
	String _debugoutdir;
};