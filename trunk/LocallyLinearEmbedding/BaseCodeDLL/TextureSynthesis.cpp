#include "Main.h"

void TextureSynthesis::Init(const AppParameters &parameters, const GaussianPyramid &exemplar, const NeighborhoodGenerator &generator, int nlevels, int reducedDimension)
{
	_debug = true;
	_debugoutdir = "texsyn-out/";
	_usepca = parameters.texsyn_usepca;

	clock_t start = clock();
	if (_usepca) InitPCA(parameters, exemplar, generator);
	InitKDTree(exemplar, generator, reducedDimension);
	Console::WriteLine(String("...initialized in ") + String(((float)(clock()-start))/CLOCKS_PER_SEC) + String(" seconds"));
}

void TextureSynthesis::InitPCA(const AppParameters &parameters, const GaussianPyramid &exemplar, const NeighborhoodGenerator &generator)
{
	Console::WriteLine("Initializing PCA...");

	_pca.Allocate(exemplar.Depth());

	const UINT neighborhoodCount = 32000;
	const UINT dimension = generator.Dimension();
	const int nRadius = generator.NeighborhoodRadius();

	for (int level = 0; level < exemplar.Depth(); level++) {

		String inputdescription = "";
		if (parameters.texsyn_usergb) inputdescription = inputdescription + "_rgb";
		if (parameters.texsyn_uselayers) inputdescription = inputdescription + "_layers";
		String cache = "../TexSynCache/" + parameters.texsyn_exemplar + "_";
		String filename = cache + String("pca_nr-") + String(nRadius) + String("_nl-") +  String(exemplar.NumLayers()) + 
			String("_lv-") + String(level) + 
			inputdescription + String(".txt");
		if (Utility::FileExists(filename)) {
			_pca[level].LoadFromFile(filename);
			Console::WriteLine(String("...loaded level ") + String(level) + String(" from file"));
		}
		else {
			const int width = exemplar[level].First().Width();
			const int height = exemplar[level].First().Height();

			Vector<const double*> neighborhoods(neighborhoodCount);

			for(UINT neighborhoodIndex = 0; neighborhoodIndex < neighborhoodCount; neighborhoodIndex++) {
				double *curNeighborhood = new double[dimension];
				bool success = false;
				while(!success) {
					Vec2i centerPt = Vec2i(rand() % (width-nRadius*2)+nRadius, rand() % (height-nRadius*2)+nRadius);
					int xCenter = centerPt.x;
					int yCenter = centerPt.y;

					success = generator.Generate(exemplar, level, xCenter, yCenter, curNeighborhood);
				}
				neighborhoods[neighborhoodIndex] = curNeighborhood;
				if (neighborhoodIndex % 1000 == 0)
					Console::WriteLine("Initialized "+String(neighborhoodIndex)+" neighborhoods");
			}

			_pca[level].InitFromDensePoints(neighborhoods, dimension);
			neighborhoods.DeleteMemory();

			// cache
			_pca[level].SaveToFile(filename);
		}
	}

}

void TextureSynthesis::InitKDTree(const GaussianPyramid &exemplar, const NeighborhoodGenerator &generator, UINT reducedDimension)
{
	_tree.Allocate(exemplar.Depth());
	_treeCoordinates.Allocate(exemplar.Depth());
	if(_usepca) _reducedDimension = reducedDimension;
	else _reducedDimension = generator.Dimension();

	const UINT dimension = generator.Dimension();
	const UINT neighbors = 5;
	double *neighborhood = new double[dimension];

	for (int level = 0; level < exemplar.Depth(); level++) {
		const int width = exemplar[level].First().Width();
		const int height = exemplar[level].First().Height();
		Vector<const double*> allNeighborhoods;

		int valid = 0;
		for(int y = 0; y < height; y++) {
			for(int x = 0; x < width; x++) {
				if(generator.Generate(exemplar, level, x, y, neighborhood)) {
					double *reducedNeighborhood = new double[_reducedDimension];
					if (_usepca) _pca[level].Transform(reducedNeighborhood, neighborhood, _reducedDimension);
					else { for (int i = 0; i < dimension; i++) reducedNeighborhood[i] = neighborhood[i]; }

					allNeighborhoods.PushEnd(reducedNeighborhood);
					_treeCoordinates[level].PushEnd(Vec2i(x,y));
					valid++;
				}
			}
		}
		Console::WriteLine(String("[ level ") + String(level) + String(" ] Building KDTree, ") + String(allNeighborhoods.Length()) + String(" neighborhoods..."));
		_tree[level].BuildTree(allNeighborhoods, _reducedDimension, neighbors);
		allNeighborhoods.DeleteMemory();
	}
	delete[] neighborhood;
}


void TextureSynthesis::Synthesize(const GaussianPyramid &rgbpyr, const GaussianPyramid &exemplar, const AppParameters &parameters, NeighborhoodGenerator &generator, double kappa)
{
	Console::WriteLine("Synthesizing texture...");

	// initialize
	int outputwidth = parameters.texsyn_outputwidth;
	int outputheight = parameters.texsyn_outputheight;
	int nradius = generator.NeighborhoodRadius();
	int pad = (nradius + 1) * (exemplar.Depth() - 1) / 2 + nradius;
	int endpad = generator.NeighborhoodRadius();
	Grid<Vec2i> coordinates(outputheight+2*endpad, outputwidth+2*endpad, Vec2i(-1,-1));
	Grid<Vec2i> upsampcoordinates(outputheight+2*endpad, outputwidth+2*endpad, Vec2i(-1,-1));

	Grid<Vec2i> prevcoordinates(outputheight+2*endpad, outputwidth+2*endpad, Vec2i(-1,-1));

	//const UINT subpass = 2; // synthesize in 2x2 squares

	int exemplarwidth = exemplar.Base().First().Width();
	int exemplarheight = exemplar.Base().First().Height();
	int excoarsewidth = exemplar[exemplar.Depth()-1].First().Width();
	int excoarseheight = exemplar[exemplar.Depth()-1].First().Height();
	int coarsewidth = outputwidth >> (exemplar.Depth()-1);
	int coarseheight = outputheight >> (exemplar.Depth()-1);
	UINT dimension = generator.Dimension();
	double *neighbourhood = new double[dimension]; 
	double *transformedNeighbourhood = new double[_reducedDimension];
	double *coherentneighbourhood = new double[dimension];
	double *transformedCohNeighbourhood = new double[_reducedDimension];

	for (int row = 0; row < coarseheight+2*pad; row++) {
		for (int col = 0; col < coarsewidth+2*pad; col++) {
			coordinates(row,col) = Vec2i((int)(((float)std::rand()/RAND_MAX)*(excoarseheight-1)), (int)(((float)std::rand()/RAND_MAX)*(excoarsewidth-1)));
		}
	}
	Console::WriteLine("exemplar [" + String(exemplarwidth) + " by " + String(exemplarheight) + "] -> output [" + String(outputwidth) + "," + String(outputheight) + "] | " + String(exemplar.NumLayers()) + " layers, dimension = " + String(_reducedDimension));

	const double CORRECTION_THRESHOLD = 5;
	const int MAX_ITERS = 3;

	int prevpaddedwidth = coarsewidth + 2 * pad, prevpaddedheight = coarseheight + 2 * pad;
	for (int level = exemplar.Depth()-1; level >= 0; level--) {
		int delta = 1 << level; 

		int width = outputwidth / delta;
		int height = outputheight / delta;
		pad = (int) ceil(nradius * (2.0 - 1 / (float)delta));
		int paddedwidth = width + 2 * pad;
		int paddedheight = height + 2 * pad;
		exemplarwidth = exemplar[level].First().Width();
		exemplarheight = exemplar[level].First().Height();
		Console::WriteLine(String("[ level ") + String(level) + String(" ] e(") + String(exemplarheight) + String(",") + String(exemplarwidth) + String(") | s(") + String(height) + String(",") + String(width) + String(")"));

		if (level < exemplar.Depth()-1) { // upsample
			int lowidth = width / 2;
			int loheight = height / 2;
			int rowoffset, coloffset, lorow, locol;
			for (int row = 0; row < paddedheight; row++) {
				rowoffset = row % 2;
				lorow = row / 2 + nradius;
				for (int col = 0; col < paddedwidth; col++) {
					coloffset = col % 2;
					locol = col / 2 + nradius;
					upsampcoordinates(row,col) = Vec2i((2*coordinates(lorow,locol).x+rowoffset) % exemplarheight, (2*coordinates(lorow,locol).y+coloffset) % exemplarwidth);
				}
			}

			for (int row = 0; row < paddedheight; row++) {
				for (int col = 0; col < paddedwidth; col++)
					coordinates(row,col) = upsampcoordinates(row,col);
			}
		}
		if (_debug) {
			WriteImage(rgbpyr, exemplar, coordinates, level, width, height, pad, String("upsamp"));
			//WriteImage(rgbpyr, exemplar, coordinates, level, paddedwidth, paddedheight, 0, String("upsamp-all"));
		}

		// synthesis
		int iteration = 0;
		double diff = FLT_MAX, d;
		while (iteration < MAX_ITERS && diff > CORRECTION_THRESHOLD) {

			for (int y = nradius; y < paddedheight-nradius; y++) {
				for (int x = nradius; x < paddedwidth-nradius; x++) {
					prevcoordinates(y,x).x = coordinates(y,x).x;
					prevcoordinates(y,x).y = coordinates(y,x).y;
				}
			}

			_coherentcount = 0;

			/*for (int sy =  0; sy < subpass; sy++) {
			for (int sx = 0; sx < subpass; sx++) {
			// int sx = 0, sy = 0, subpass = 1;*/

			for (int y = nradius; y < paddedheight-nradius; y++) {
				for (int x = nradius; x < paddedwidth-nradius; x++) {
					Vec2i pt = BestMatch(exemplar, generator, coordinates, level, x, y, kappa,
						neighbourhood, transformedNeighbourhood, paddedwidth, paddedheight,
						coherentneighbourhood, transformedCohNeighbourhood);
					coordinates(y,x) = pt;
				}
			}

			/*}
			}*/

			Console::WriteLine(String(" coherent count = ") + String(_coherentcount) + String(" of ") + String((paddedwidth-2*nradius)*(paddedheight-2*nradius)));

			diff = 0;
			for (int y = nradius; y < paddedheight-nradius; y++) {
				for (int x = nradius; x < paddedwidth-nradius; x++) {
					for (int k = 0; k < exemplar.NumLayers(); k++) {
						d = 255.f*exemplar[level][k].pixelWeights(coordinates(y,x).y,coordinates(y,x).x) - 255.f*exemplar[level][k].pixelWeights(prevcoordinates(y,x).y,prevcoordinates(y,x).x);
						diff = d * d;
					}
				}
			}
			Console::WriteLine(String("[ level ") + String(level) + String(", iteration ") + String(iteration) + String(" ] diff ") + String(diff));
			if (_debug) {
				WriteImage(rgbpyr, exemplar, coordinates, level, width, height, pad, String("correct-")+String(iteration));
				//WriteImage(rgbpyr, exemplar, coordinates, level, paddedwidth, paddedheight, 0, String("correct-all-")+String(iteration));
			}
			prevpaddedwidth = width + 2 * pad;
			prevpaddedheight = height + 2 * pad;
			iteration++;
		}
	}

	delete [] neighbourhood;
	delete [] transformedNeighbourhood;
	delete [] coherentneighbourhood;
	delete [] transformedCohNeighbourhood;
	Console::WriteLine("Done!");
}

Vec2i TextureSynthesis::BestMatch(const GaussianPyramid &exemplar, NeighborhoodGenerator &generator, const Grid<Vec2i> &coordinates, int level, int x, int y, double kappa,
								  double *neighbourhood, double *transformedNeighbourhood, int width, int height,
								  double *coherentneighbourhood, double *transformedCohNeighbourhood)
{
	Vec2i approxPt, coherentPt;

/*	if (!generator.Generate(exemplar, coordinates, level, width, height, x, y, neighbourhood))
		Console::WriteLine("");*/
	generator.Generate(exemplar, coordinates, level, width, height, x, y, neighbourhood);

	if (_usepca) _pca[level].Transform(transformedNeighbourhood, neighbourhood, _reducedDimension);
	else transformedNeighbourhood = neighbourhood;

	double nearestDist = BestApproximateMatch(exemplar, generator, level, approxPt, transformedNeighbourhood);
	double nearestCoherentDist = BestCoherentMatch(exemplar, generator, coordinates, level, width, height,
		x, y, coherentPt, transformedNeighbourhood, coherentneighbourhood, transformedCohNeighbourhood);

	if (nearestCoherentDist < (nearestDist + kappa)) {
		_coherentcount++;
		return coherentPt;
	}
	return approxPt;
}

double TextureSynthesis::BestCoherentMatch(const GaussianPyramid &exemplar, NeighborhoodGenerator &generator, const Grid<Vec2i> &coordinates, 
										   int level, int width, int height, int x, int y, Vec2i &outPt,
										   double *transformedNeighbourhood, double *coherentneighbourhood, double *transformedCohNeighbourhood)
{
	int xcoord, ycoord;
	double mindist = FLT_MAX, d;
	for (int j = -1; j <= 0; j++) {
		for (int i = -1; i <= 1; i++) {
			if (i == 0 && j == 0) break; // since scanline order

			if (y+j >= 0 && y+j < height && x+i >= 0 && x+i < width) {
				xcoord = coordinates(y+j,x+i).x - i;
				ycoord = coordinates(y+j,x+i).y - j;

				int exh = exemplar[level].First().Height();
				int exw = exemplar[level].First().Width();
				if (ycoord >= 0 && ycoord < exemplar[level].First().Height() && xcoord >= 0 && xcoord < exemplar[level].First().Width()) {
					if (generator.Generate(exemplar, level, xcoord, ycoord, coherentneighbourhood)) {
						if (_usepca) _pca[level].Transform(transformedCohNeighbourhood, coherentneighbourhood, _reducedDimension);
						else transformedCohNeighbourhood = coherentneighbourhood;

						d = NeighborhoodDistance(transformedNeighbourhood, transformedCohNeighbourhood, _reducedDimension);
						if (d < mindist) {
							mindist = d;
							outPt.x = xcoord;
							outPt.y = ycoord;
						}
					}
				}
			}
		}
	}

	return mindist;
}

double TextureSynthesis::BestApproximateMatch(const GaussianPyramid &exemplar, NeighborhoodGenerator &generator, int level, Vec2i &outPt,
											  double *transformedNeighbourhood)
{
	Vector<UINT> indices(1);

	//find nearest pixel neighborhood			
	_tree[level].KNearest(transformedNeighbourhood, 1, indices, 0.0f);
	Vec2i sourceCoordinate = _treeCoordinates[level][indices[0]];

	outPt.x = sourceCoordinate.x;
	outPt.y = sourceCoordinate.y;

	double* matchedNeighbourhood = _tree[level].GetDataPoint(indices[0]);

	return NeighborhoodDistance(transformedNeighbourhood, matchedNeighbourhood, _reducedDimension);
}

double TextureSynthesis::NeighborhoodDistance(double* neighborhoodA, double* neighborhoodB, UINT dimension)
{
	double result = 0;
	for (UINT i=0; i<dimension; i++)
		result += Math::Square(neighborhoodA[i]-neighborhoodB[i]);
	return result;
}


void TextureSynthesis::WriteImage(const GaussianPyramid &rgbpyr, const GaussianPyramid &exemplar, const Grid<Vec2i> &coordinates, int level, int width, int height, int pad, String label)
{
	UINT exemplarwidth = exemplar[level].First().Width();
	UINT exemplarheight = exemplar[level].First().Height();
	Bitmap coordimage(height, width);
	for (int row = 0; row < height; row++) { // coordinates
		for (int col = 0; col < width; col++)
			coordimage[row][col] = RGBColor(Vec3f((float)coordinates(row+pad,col+pad).y/exemplarheight, (float)coordinates(row+pad,col+pad).x/exemplarwidth, 0));
	}
	coordimage.SavePNG(_debugoutdir + String("coord-") + String(level) + String("_") + label + String(".png"));
	Console::WriteLine(_debugoutdir + String("coord-") + String(level) + String("_") + label + String(".png"));
	for (int row = 0; row < height; row++) { // image
		for (int col = 0; col < width; col++) {

			/*if (coordinates(row+pad,col+pad).x < 0 || coordinates(row+pad,col+pad).x >= rgbpyr[level][0].pixelWeights.Cols()) {
				int tx = coordinates(row,col).x;
				int bound = rgbpyr[level][0].pixelWeights.Cols();
				Console::WriteLine("xxx");
			}
			if (coordinates(row+pad,col+pad).y < 0 || coordinates(row+pad,col+pad).y >= rgbpyr[level][0].pixelWeights.Rows()) {
				int ty = coordinates(row,col).y;
				int bound = rgbpyr[level][0].pixelWeights.Rows();
				Console::WriteLine("yyy");
			}*/

			coordimage[row][col] = RGBColor(Vec3f(rgbpyr[level][0].pixelWeights(coordinates(row+pad,col+pad).y, coordinates(row+pad,col+pad).x),
				rgbpyr[level][1].pixelWeights(coordinates(row+pad,col+pad).y, coordinates(row+pad,col+pad).x),
				rgbpyr[level][2].pixelWeights(coordinates(row+pad,col+pad).y, coordinates(row+pad,col+pad).x)));
		}
	}
	coordimage.SavePNG(_debugoutdir + String("image-") + String(level) + String("_") + label + String(".png"));
	Console::WriteLine(_debugoutdir + String("image-") + String(level) + String("_") + label + String(".png"));
}