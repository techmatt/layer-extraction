#include "Main.h"


void LayerTextureSynthesizer::Synthesize(const PixelLayerSet &layers, const PixelLayerSet &rgb, const AppParameters &parameters,
										 NeighborhoodGenerator &generator, Vector<int> order)
{
	_outputdir = "texsyn-out/LAYER/";
	PixelLayerSet output;
	Vector< Grid<Vec2i> > coordinates;

	_kappa = parameters.texsyn_kappa;
	_dimension = parameters.texsyn_neighbourhoodsize * 2 + 1;
	_dimension *=  _dimension;

	for (int iter = 0; iter < (int)order.Length(); iter++) {
		Synthesize(layers, rgb, parameters, generator, order, iter, output, coordinates);
	}

	// compose
	Bitmap synthesized(parameters.texsyn_outputwidth, parameters.texsyn_outputheight);
	for (int y = 0; y < parameters.texsyn_outputheight; y++) {
		for (int x = 0; x < parameters.texsyn_outputwidth; x++) {
			Vec3f v(0,0,0);
			for (int k = 0; k < (int)output.Length(); k++) 
				v += output[k].color * (float)output[k].pixelWeights(y,x);
			synthesized[y][x] = RGBColor(v);
		}
	}
	synthesized.SavePNG(_outputdir + "output.png");
}


void LayerTextureSynthesizer::Synthesize(const PixelLayerSet &layers, const PixelLayerSet &rgb, const AppParameters &parameters,
										 NeighborhoodGenerator &generator, Vector<int> order, int iteration, 
										 PixelLayerSet &synthesized, Vector< Grid<Vec2i> > &coordinateset)
{
	Console::WriteLine("...initializing iteration " + String(iteration) + " (layer " + String(order[iteration]) + ")");
	_reducedDimension = parameters.texsyn_pcadim;
	PCA<double> pca;
	ComputePCA(parameters, layers, generator, order, iteration, pca);
	Vector<Vec2i> treeCoordinates;
	KDTree kdtree;
	ConstructKDTree(pca, layers, generator, order, iteration, treeCoordinates, kdtree);

	Console::WriteLine("Synthesizing layer " + String(order[iteration]) + "...");
	String cond = String(order[iteration]);
	for (int i = iteration-1; i >= 0; i--) cond += "-" + String(order[i]);

	int outputwidth = parameters.texsyn_outputwidth;
	int outputheight = parameters.texsyn_outputheight;
	int nradius = generator.NeighborhoodRadius();
	int paddedwidth = outputwidth + 2 * nradius;
	int paddedheight = outputheight + 2 * nradius;

	int exemplarwidth = layers.First().Width();
	int exemplarheight = layers.First().Height();

	UINT dimension = _dimension * (iteration+1);
	double *neighbourhood = new double[dimension]; 
	double *transformedNeighbourhood = new double[_reducedDimension];
	double *coherentneighbourhood = new double[dimension];
	double *transformedCohNeighbourhood = new double[_reducedDimension];

	PixelLayer output(layers[order[iteration]].color, paddedwidth, paddedheight);
	Grid<Vec2i> coordinates(paddedheight, paddedwidth, Vec2i(-1,-1));
	// initialize to random skip by skip blocks
	const int skip = parameters.texsyn_initrandsize;
	int rx, ry;
	for (int row = 0; row < paddedheight; row+=skip) {
		for (int col = 0; col < paddedwidth; col+=skip) {
			ry = (int)(((float)std::rand()/RAND_MAX)*(exemplarheight-1-skip));
			rx = (int)(((float)std::rand()/RAND_MAX)*(exemplarwidth-1-skip));
			for (int j = 0; j < skip; j++) {
				for (int i = 0; i < skip; i++)  {
					if (row+j < paddedheight && col+i < paddedwidth) {
						coordinates(row+j,col+i) = Vec2i(rx+i, ry+j);
						output.pixelWeights(row+j,col+i) = layers[order[iteration]].pixelWeights(ry+j,rx+i);
					}
				}
			}
		}
	}
	synthesized.PushEnd(output);
	coordinateset.PushEnd(coordinates);

	Write(layers, synthesized, coordinateset, outputwidth, outputheight, nradius, cond+"_init");

	// synthesis
	const double CORRECTION_THRESHOLD = 10;
	const int MAX_ITERS = 3;
	Grid<double> prev(paddedheight, paddedwidth);
	Grid<Vec2i> prevcoords(paddedheight, paddedwidth);
	int iter = 0;
	double diff = FLT_MAX, d, prevdiff = FLT_MAX;
	bool done = false;
	while (iter < MAX_ITERS && diff > CORRECTION_THRESHOLD && !done) {

		for (int y = nradius; y < outputheight+nradius; y++) {
			for (int x = nradius; x < outputwidth+nradius; x++) {
				prev(y,x) = synthesized[iteration].pixelWeights(y,x);
				prevcoords(y,x).x = coordinateset[iteration](y,x).x;
				prevcoords(y,x).y = coordinateset[iteration](y,x).y;
			}
		}

		_coherentcount = 0;

		for (int y = 0; y < paddedheight; y++) {
			for (int x = 0; x < paddedwidth; x++) {
				Vec2i pt = BestMatch(layers, pca, kdtree, treeCoordinates, generator, synthesized, coordinateset, order, iteration, x, y,
					neighbourhood, transformedNeighbourhood, paddedwidth, paddedheight,
					coherentneighbourhood, transformedCohNeighbourhood);
				coordinateset[iteration](y,x) = pt;
				synthesized[iteration].pixelWeights(y,x) = layers[order[iteration]].pixelWeights(pt.y, pt.x);
			}
		}

		Console::WriteLine(String(" coherent count = ") + String(_coherentcount) + String(" of ") + String(paddedwidth*paddedheight));

		diff = 0;
		for (int y = nradius; y < outputheight+nradius; y++) {
			for (int x = nradius; x < outputwidth+nradius; x++) {
				d = prev(y,x) - synthesized[iteration].pixelWeights(y,x);
				diff += d * d;
			}
		}
		Console::WriteLine("[ correction pass " + String(iter) + " ] diff " + String(diff));
		if (diff > prevdiff) { // stop
			Console::WriteLine(" --- backtracking...");
			for (int y = nradius; y < outputheight+nradius; y++) {
				for (int x = nradius; x < outputwidth+nradius; x++) {
					synthesized[iteration].pixelWeights(y,x) = prev(y,x);
					coordinateset[iteration](y,x).x = prevcoords(y,x).x;
					coordinateset[iteration](y,x).y = prevcoords(y,x).y;
				}
			}
			done = true;
		}
		prevdiff = diff;

		Write(layers, synthesized, coordinateset, outputwidth, outputheight, nradius, cond+"_correct-"+String(iter));

		iter++;
	}
	Console::WriteLine("Done!");
}

Vec2i LayerTextureSynthesizer::BestMatch(const PixelLayerSet &layers, PCA<double> &pca, KDTree &kdtree, const Vector<Vec2i> &treeCoordinates,
										 NeighborhoodGenerator &generator, const PixelLayerSet &synthesized, const Vector< Grid<Vec2i> > &coordinateset, 
										 Vector<int> order, int iteration, int x, int y, 
										 double *neighbourhood, double *transformedNeighbourhood, int width, int height,
										 double *coherentneighbourhood, double *transformedCohNeighbourhood)
{
	Vec2i approxPt, coherentPt;

	generator.Generate(synthesized, coordinateset, order, iteration, width, height, x, y, neighbourhood);

	pca.Transform(transformedNeighbourhood, neighbourhood, _reducedDimension);

	double nearestDist = BestApproximateMatch(kdtree, treeCoordinates, layers, generator, approxPt, transformedNeighbourhood);

	/*double *test = new double[_dimension*(iteration+1)];
	double *ttest = new double[_reducedDimension];
	generator.Generate(layers, order, iteration, approxPt.x, approxPt.y, test);
	pca.Transform(ttest, test, _reducedDimension);
	Vector<double> match(_reducedDimension);
	for (int i = 0; i < _reducedDimension; i++)
	match[i] = ttest[i];
	Vector<double> query(_reducedDimension);
	for (int i = 0; i < _reducedDimension; i++)
	query[i] = transformedNeighbourhood[i];
	double dist = NeighborhoodDistance(transformedNeighbourhood, ttest, _reducedDimension);
	double rdist = NeighborhoodDistance(neighbourhood, test, _dimension*(iteration+1));
	Vec2i testpt(43,33);
	generator.Generate(layers, order, iteration, testpt.x, testpt.y, test);
	pca.Transform(ttest, test, _reducedDimension);
	double dist2 = NeighborhoodDistance(transformedNeighbourhood, ttest, _reducedDimension);
	double rdist2 = NeighborhoodDistance(neighbourhood, test, _dimension*(iteration+1));
	VisualizeMatches(layers, synthesized, order, iteration, generator.NeighborhoodRadius(), Vec2i(x,y), testpt);
	VisualizeMatches(layers, synthesized, order, iteration, generator.NeighborhoodRadius(), Vec2i(x,y), approxPt);
	*/

	if (_kappa == 0) return approxPt;

	double nearestCoherentDist = BestCoherentMatch(layers, pca, generator, coordinateset, order, iteration, width, height,
		x, y, coherentPt, transformedNeighbourhood, coherentneighbourhood, transformedCohNeighbourhood);

	double kappa = _kappa / (iteration+1);
	if (nearestCoherentDist < nearestDist * (1 + 0.5*kappa)) {
		_coherentcount++;
		return coherentPt;
	}
	return approxPt;
}

double LayerTextureSynthesizer::BestCoherentMatch(const PixelLayerSet &layers, PCA<double> &pca, NeighborhoodGenerator &generator, const Vector< Grid<Vec2i> > &coordinateset, 
												  Vector<int> order, int iteration, int width, int height, int x, int y, Vec2i &outPt, double *transformedNeighbourhood,
												  double *coherentneighbourhood, double *transformedCohNeighbourhood)
{
	int xcoord, ycoord;
	double mindist = FLT_MAX, d;
	for (int j = -1; j <= 0; j++) {
		for (int i = -1; i <= 1; i++) {
			if (i == 0 && j == 0) break; // since scanline order

			if (y+j >= 0 && y+j < height && x+i >= 0 && x+i < width) {
				xcoord = coordinateset[iteration](y+j,x+i).x - i;
				ycoord = coordinateset[iteration](y+j,x+i).y - j;

				int exh = layers.First().Height();
				int exw = layers.First().Width();
				if (ycoord >= 0 && ycoord < exh && xcoord >= 0 && xcoord < exw) {
					if (generator.Generate(layers, order, iteration, xcoord, ycoord, coherentneighbourhood)) {
						pca.Transform(transformedCohNeighbourhood, coherentneighbourhood, _reducedDimension);

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

double LayerTextureSynthesizer::BestApproximateMatch(KDTree &kdtree, const Vector<Vec2i> &treeCoordinates, const PixelLayerSet &layers,
													 NeighborhoodGenerator &generator, Vec2i &outPt, double *transformedNeighbourhood)
{
	Vector<UINT> indices(1);

	//find nearest pixel neighborhood			
	kdtree.KNearest(transformedNeighbourhood, 1, indices, 0.0f);
	Vec2i sourceCoordinate = treeCoordinates[indices[0]];

	outPt.x = sourceCoordinate.x;
	outPt.y = sourceCoordinate.y;

	double* matchedNeighbourhood = kdtree.GetDataPoint(indices[0]);

	return NeighborhoodDistance(transformedNeighbourhood, matchedNeighbourhood, _reducedDimension);
}

double LayerTextureSynthesizer::NeighborhoodDistance(double* neighborhoodA, double* neighborhoodB, UINT dimension)
{
	double result = 0;
	for (UINT i=0; i<dimension; i++)
		result += Math::Square(neighborhoodA[i]-neighborhoodB[i]);
	return result;
}

void LayerTextureSynthesizer::ComputePCA(const AppParameters &parameters, const PixelLayerSet &layers, const NeighborhoodGenerator &generator,
										 Vector<int> order, int iteration, PCA<double> &pca)
{
	const UINT neighborhoodCount = 32000;
	const UINT dimension = _dimension * (iteration+1);
	int nRadius = generator.NeighborhoodRadius();

	String cache = "../TexSynCache/layersynth/" + parameters.texsyn_exemplar + "_";
	String cond = "";
	for (int i = 0; i < iteration; i++)
		cond += "-" + String(order[i]);
	String filename = cache + String("pca_nr-") + String(nRadius) + String("_layer-") +  String(order[iteration]) + String("_cond") + cond + String(".txt");
	if (Utility::FileExists(filename)) {
		pca.LoadFromFile(filename);
		Console::WriteLine(String("...loaded iteration ") + String(iteration) + String(" from file"));
	}
	else {
		const int width = layers.First().Width();
		const int height = layers.First().Height();

		Vector<const double*> neighborhoods(neighborhoodCount);

		for(UINT neighborhoodIndex = 0; neighborhoodIndex < neighborhoodCount; neighborhoodIndex++) {
			double *curNeighborhood = new double[dimension];
			bool success = false;
			while(!success) {
				Vec2i centerPt = Vec2i(rand() % (width-nRadius*2)+nRadius, rand() % (height-nRadius*2)+nRadius);
				int xCenter = centerPt.x;
				int yCenter = centerPt.y;

				success = generator.Generate(layers, order, iteration, xCenter, yCenter, curNeighborhood);
			}


			neighborhoods[neighborhoodIndex] = curNeighborhood;
			if (neighborhoodIndex % 1000 == 0)
				Console::WriteLine("Initialized "+String(neighborhoodIndex)+" neighborhoods");
		}

		pca.InitFromDensePoints(neighborhoods, dimension);
		neighborhoods.DeleteMemory();

		// cache
		pca.SaveToFile(filename);
	}
}

void LayerTextureSynthesizer::ConstructKDTree(PCA<double> &pca, const PixelLayerSet &layers, const NeighborhoodGenerator &generator,
											  Vector<int> order, int iteration, Vector<Vec2i> &treeCoordinates, KDTree &kdtree)
{
	const UINT dimension = _dimension * (iteration+1);
	const UINT neighbors = 5;
	double *neighborhood = new double[dimension];

	const int width = layers.First().Width();
	const int height = layers.First().Height();
	Vector<const double*> allNeighborhoods;

	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			if(generator.Generate(layers, order, iteration, x, y, neighborhood)) {
				double *reducedNeighborhood = new double[_reducedDimension];
				pca.Transform(reducedNeighborhood, neighborhood, _reducedDimension);

				allNeighborhoods.PushEnd(reducedNeighborhood);
				treeCoordinates.PushEnd(Vec2i(x,y));
			}
		}
	}
	Console::WriteLine(String("[ iter ") + String(iteration) + String(" ] Building KDTree, ") + String(allNeighborhoods.Length()) + String(" neighborhoods..."));
	kdtree.BuildTree(allNeighborhoods, _reducedDimension, neighbors);
	allNeighborhoods.DeleteMemory();
	delete[] neighborhood;
}

void LayerTextureSynthesizer::Write(const PixelLayerSet &layers, PixelLayerSet &synth, const Vector< Grid<Vec2i> > &coordinateset, int width, int height, int pad, String cond) const
{
	int idx = synth.Length() - 1;
	UINT exemplarwidth = layers.First().Width();
	UINT exemplarheight = layers.First().Height();
	Bitmap coordimage(height, width);
	for (int row = 0; row < height; row++) { // coordinates
		for (int col = 0; col < width; col++)
			coordimage[row][col] = RGBColor(Vec3f((float)coordinateset[idx](row+pad,col+pad).y/exemplarheight, (float)coordinateset[idx](row+pad,col+pad).x/exemplarwidth, 0));
	}
	coordimage.SavePNG(_outputdir + String("coord_") + cond + String(".png"));
	PixelLayer p(synth[idx].color, width, height);
	for (int row = 0; row < height; row++) { // layer 
		for (int col = 0; col < width; col++)
			p.pixelWeights(row, col) = synth[idx].pixelWeights(row+pad, col+pad);
	}
	p.SavePNG(_outputdir + String("image_") + cond + String(".png"));
}

void LayerTextureSynthesizer::VisualizeMatches(const PixelLayerSet &layers, const PixelLayerSet &synthlayers, Vector<int> order, int iteration, int nradius,
											   const Vec2i &pt, const Vec2i &matchpt) const
{
	RGBColor mark(255,0,0);
	// copy
	Vector<Bitmap> exemplar(iteration+1);
	Vector<Bitmap> synth(iteration+1);
	for (int i = 0; i <= iteration; i++) {
		exemplar[i].Allocate(layers[order[i]].Height(), layers[order[i]].Width());
		synth[i].Allocate(synthlayers[i].Height(), synthlayers[i].Width());
		for (int y = 0; y < layers[order[i]].Height(); y++) 
			for (int x = 0; x < layers[order[i]].Width(); x++)
				exemplar[i][y][x] = RGBColor(Vec3f(layers[order[i]].pixelWeights(y,x),layers[order[i]].pixelWeights(y,x),layers[order[i]].pixelWeights(y,x)));
		for (int y = 0; y < synthlayers[i].Height(); y++) 
			for (int x = 0; x < synthlayers[i].Width(); x++)
				synth[i][y][x] = RGBColor(Vec3f(synthlayers[i].pixelWeights(y,x),synthlayers[i].pixelWeights(y,x),synthlayers[i].pixelWeights(y,x)));
		// draw box around neighbourhood
		int ystart = max(0, matchpt.y-nradius-1);
		int yend = min(layers[order[i]].Height()-1, matchpt.y+nradius+1);
		int xstart = max(0, matchpt.x-nradius-1);
		int xend = min(layers[order[i]].Width()-1, matchpt.x+nradius+1);
		for (int y = ystart; y <= yend; y++) {
			exemplar[i][y][xstart] = mark;
			exemplar[i][y][xend] = mark;
		}
		for (int x = xstart; x <= xend; x++) {
			exemplar[i][ystart][x] = mark;
			exemplar[i][yend][x] = mark;
		}

		ystart = max(0, pt.y-nradius-1);
		yend = min(synthlayers[i].Height()-1, pt.y+nradius+1);
		xstart = max(0, pt.x-nradius-1);
		xend = min(synthlayers[i].Width()-1, pt.x+nradius+1);
		for (int y = ystart; y <= yend; y++) {
			synth[i][y][xstart] = mark;
			synth[i][y][xend] = mark;
		}
		for (int x = xstart; x <= xend; x++) {
			synth[i][ystart][x] = mark;
			synth[i][yend][x] = mark;
		}
		// write out
		Console::WriteLine(_outputdir+"debug/exemp-"+String(matchpt.x)+"-"+String(matchpt.y)+".png");
		exemplar[i].SavePNG(_outputdir+"debug/exemp-"+String(matchpt.x)+"-"+String(matchpt.y)+".png");
		Console::WriteLine(_outputdir+"debug/synth-"+String(pt.x)+"-"+String(pt.y)+".png");
		synth[i].SavePNG(_outputdir+"debug/synth-"+String(pt.x)+"-"+String(pt.y)+".png");

		PixelLayer exneighbourhood(layers[order[i]].color, nradius*2+1, nradius*2+1);
		PixelLayer syneighbourhood(synthlayers[i].color, nradius*2+1, nradius*2+1);
		PixelLayer diff(Vec3f(1,1,1), nradius*2+1, nradius*2+1);
		for (int yoffset = -nradius; yoffset <= nradius; yoffset++) {
			for (int xoffset = -nradius; xoffset <= nradius; xoffset++) {
				if (matchpt.y+yoffset >= 0 && matchpt.y+yoffset < (int)exemplar[i].Height() &&
					matchpt.x+xoffset >= 0 && matchpt.x+xoffset < (int)exemplar[i].Width())
					exneighbourhood.pixelWeights(yoffset+nradius,xoffset+nradius) = layers[order[iteration]].pixelWeights(matchpt.y+yoffset,matchpt.x+xoffset);
				else
					exneighbourhood.pixelWeights(yoffset+nradius,xoffset+nradius) = 0;
				if (pt.y+yoffset >= 0 && pt.y+yoffset < (int)synth[i].Height() &&
					pt.x+xoffset >= 0 && pt.x+xoffset < (int)synth[i].Width())
					syneighbourhood.pixelWeights(yoffset+nradius,xoffset+nradius) = synthlayers[iteration].pixelWeights(pt.y+yoffset,pt.x+xoffset);
				else
					syneighbourhood.pixelWeights(yoffset+nradius,xoffset+nradius) = 0;
				diff.pixelWeights(yoffset+nradius,xoffset+nradius) = abs(exneighbourhood.pixelWeights(yoffset+nradius,xoffset+nradius) - syneighbourhood.pixelWeights(yoffset+nradius,xoffset+nradius));
			}
		}
		Console::WriteLine(_outputdir+"debug/exemp-neighbourhood-"+String(matchpt.x)+"-"+String(matchpt.y)+".png");
		exneighbourhood.SavePNG(_outputdir+"debug/exemp-neighbourhood-"+String(matchpt.x)+"-"+String(matchpt.y)+".png", false);
		Console::WriteLine(_outputdir+"debug/synth-neighbourhood-"+String(pt.x)+"-"+String(pt.y)+".png");
		syneighbourhood.SavePNG(_outputdir+"debug/synth-neighbourhood-"+String(pt.x)+"-"+String(pt.y)+".png", false);
		Console::WriteLine(_outputdir+"debug/diff-neighbourhood-s-"+String(pt.x)+"-"+String(pt.y)+"-m-"+String(matchpt.x)+"-"+String(matchpt.y)+".png");
		syneighbourhood.SavePNG(_outputdir+"debug/diff-neighbourhood-s-"+String(pt.x)+"-"+String(pt.y)+"-m-"+String(matchpt.x)+"-"+String(matchpt.y)+".png", false);

		/*String diffname = _outputdir+"debug/diff-neighbourhood-s-"+String(pt.x)+"-"+String(pt.y)+"-m-"+String(matchpt.x)+"-"+String(matchpt.y)+".txt";
		ofstream File(diffname.CString());
		PersistentAssert(!File.fail(), "Failed to open file");
		for (int i = 0; i < _parameters.texsyn_klayers; i++)
		File << palette[i].TabSeparatedString() << endl;*/
	}
}