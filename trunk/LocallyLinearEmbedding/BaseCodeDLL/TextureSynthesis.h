
class TextureSynthesis
{
public:

	void Init(const AppParameters &parameters, const GaussianPyramid &exemplar, const NeighborhoodGenerator &generator, int nlevels, int reducedDimension);

	void Synthesize(const GaussianPyramid &rgbpyr, const GaussianPyramid &exemplar, const AppParameters &parameters, NeighborhoodGenerator &generator);

	Vec2i BestMatchP(int threadIndex, const GaussianPyramid *exemplar, NeighborhoodGenerator *generator, const Grid<Vec2i> *coordinates,
				    int level, int x, int y, int width, int height);
	
	String CreateOutputDirectory(const AppParameters &parameters);

private:
    void InitPCA(const AppParameters &parameters, const GaussianPyramid &exemplar, const NeighborhoodGenerator &generator);
	void InitKDTree(const GaussianPyramid &exemplar, const NeighborhoodGenerator &generator, UINT reducedDimension);

	Vec2i BestMatch(const GaussianPyramid &exemplar, NeighborhoodGenerator &generator, const Grid<Vec2i> &coordinates,
				    int level, int x, int y,
					double *neighbourhood, double *transformedNeighbourhood, int width, int height,
					double *coherentneighbourhood, double *transformedCohNeighbourhood);
	double BestApproximateMatch(int threadindex, const GaussianPyramid &exemplar, NeighborhoodGenerator &generator, int level, Vec2i &outPt,
											  double *transformedNeighbourhood);
	double BestCoherentMatch(const GaussianPyramid &exemplar, NeighborhoodGenerator &generator, const Grid<Vec2i> &coordinates,
							 int level, int width, int height, int x, int y, Vec2i &outPt, double *transformedNeighbourhood,
							 double *coherentneighbourhood, double *transformedCohNeighbourhood);

	double NeighborhoodDistance(double* neighborhoodA, double* neighborhoodB, UINT dimension);


	void WriteImage(const GaussianPyramid &rgbpyr, const GaussianPyramid &exemplar, const Grid<Vec2i> &coordinates, int level, int width, int height, int pad, String label);


	UINT _reducedDimension;
	Vector< PCA<double> > _pca;
	Vector< Vector<KDTree> > _trees;
	Vector< Vector<Vec2i> > _treeCoordinates;
	int _nthreads; // multithreading
	double _kappa; // coherence parameter

	int _coherentcount;
	
	bool _debug;
	String _outdir;
};


struct TexSynTask : public WorkerThreadTask
{
    void Run(UINT threadIndex, ThreadLocalStorage *threadLocalStorage);
    TextureSynthesis *synthesizer;
    Vec2i pixel;
	Vec2i dimensions;
	int level;
    const GaussianPyramid *exemplar;
    NeighborhoodGenerator *generator;
	Grid<Vec2i> *coordinates;
};