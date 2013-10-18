#include "Main.h"
#include "LayerTextureSynthesizer.h"


void LayerTextureSynthesizer::Synthesize(const PixelLayerSet &layers, const PixelLayerSet &rgb, const AppParameters &parameters,
										 NeighborhoodGenerator &generator, double kappa, Vector<int> order)
{
	_outputdir = "texsyn-out/";
	PixelLayerSet output;
	Vector< Grid<Vec2i> > coordinates;

	
	for (int iter = 0; iter < (int)order.Length(); iter++) {
		Synthesize(layers, rgb, parameters, generator, kappa, order, iter, output, coordinates);
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
											   NeighborhoodGenerator &generator, double kappa, Vector<int> order, int iteration, 
											   PixelLayerSet &synthesized, Vector< Grid<Vec2i> > &coordinateset)
{
	Console::WriteLine("...initializing iteration " + String(iteration) + " (layer " + String(order[iteration]) + ")");
	_reducedDimension = parameters.texsyn_pcadim;
	PCA<double> pca = ComputePCA(parameters, layers, generator, order, iteration);
	Vector<Vec2i> treeCoordinates;
	KDTree kdtree = ConstructKDTree(pca, layers, generator, order, iteration, treeCoordinates);

	Console::WriteLine("Synthesizing layer " + String(order[iteration]) + "...");

	int outputwidth = parameters.texsyn_outputwidth;
	int outputheight = parameters.texsyn_outputheight;
	int nradius = generator.NeighborhoodRadius();

	int exemplarwidth = layers.First().Width();
	int exemplarheight = layers.First().Height();

	UINT dimension = generator.Dimension();
	double *neighbourhood = new double[dimension]; 
	double *transformedNeighbourhood = new double[_reducedDimension];
	double *coherentneighbourhood = new double[dimension];
	double *transformedCohNeighbourhood = new double[_reducedDimension];

	PixelLayer output(layers[order[iteration]].color, outputwidth, outputheight);
	Grid<Vec2i> coordinates(outputheight+2*nradius, outputwidth+2*nradius, Vec2i(-1,-1));
	// initialize to random skip by skip blocks
	const int skip = parameters.texsyn_initrandsize;
	int rx, ry;
	for (int row = 0; row < outputheight+2*nradius; row+=skip) {
		for (int col = 0; col < outputwidth+2*nradius; col+=skip) {
			rx = (int)(((float)std::rand()/RAND_MAX)*(exemplarheight-1-skip));
			ry = (int)(((float)std::rand()/RAND_MAX)*(exemplarwidth-1-skip));
			for (int j = 0; j < skip; j++) {
				for (int i = 0; i < skip; i++) 
					if (row+j < outputheight+2*nradius && col+i < outputwidth+2*nradius)
						coordinates(row+j,col+i) = Vec2i(rx+i, ry+j);
			}
		}
	}
	synthesized.PushEnd(output);
	coordinateset.PushEnd(coordinates);

	// synthesis
	const double CORRECTION_THRESHOLD = 10;
	const int MAX_ITERS = 3;
	Grid<double> prev(outputheight+2*nradius, outputwidth+2*nradius);
	Grid<Vec2i> prevcoords(outputheight+2*nradius, outputwidth+2*nradius);
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

		for (int y = nradius; y < outputheight+nradius; y++) {
			for (int x = nradius; x < outputwidth+nradius; x++) {
				/*Vec2i pt = BestMatch(layers, pca, kdtree, generator, synthesized, order, iteration, x, y, kappa,
				neighbourhood, transformedNeighbourhood, outputwidth, outputheight,
				coherentneighbourhood, transformedCohNeighbourhood);
				coordinates(y,x) = pt;
				output.pixelWeights(y,x) = layers[order[iteration]].pixelWeights(pt.y, pt.x);*/
			}
		}

		Console::WriteLine(String(" coherent count = ") + String(_coherentcount) + String(" of ") + String(outputwidth*outputheight));

		diff = 0;
		for (int y = nradius; y < outputheight+nradius; y++) {
			for (int x = nradius; x < outputwidth+nradius; x++) {
				d = prev(y,x) - synthesized[iteration].pixelWeights(y,x);
				diff += d * d;
			}
		}
		Console::WriteLine(String("[ correction pass ") + String(iter) + String(" ] diff ") + String(diff));
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

		String cond = String(order[iteration]);
		for (int i = iteration-1; i >= 0; i--) cond += "-" + String(order[i]);
		Write(layers, synthesized, coordinateset, outputwidth, outputheight, nradius, cond);

		iter++;
	}

}

Vec2i LayerTextureSynthesizer::BestMatch(const PixelLayerSet &layers, PCA<double> &pca, KDTree &kdtree, const Vector<Vec2i> &treeCoordinates,
										 NeighborhoodGenerator &generator, const Grid<Vec2i> &coordinates, 
										 Vector<int> order, int iteration, int x, int y, double kappa,
										 double *neighbourhood, double *transformedNeighbourhood, int width, int height,
										 double *coherentneighbourhood, double *transformedCohNeighbourhood)
{
	Vec2i approxPt, coherentPt;

	//generator.Generate(layers, coordinates, order, iteration, width, height, x, y, neighbourhood);

	pca.Transform(transformedNeighbourhood, neighbourhood, _reducedDimension);

	//double nearestDist = BestApproximateMatch(kdtree, treeCoordinates, layers, generator, approxPt, transformedNeighbourhood);
	/*if (kappa == 0) return approxPt;

	double nearestCoherentDist = BestCoherentMatch(layers, pca, generator, coordinates, order, iteration, width, height,
		x, y, coherentPt, transformedNeighbourhood, coherentneighbourhood, transformedCohNeighbourhood);

	if (nearestCoherentDist < nearestDist * (1 + 0.5*kappa)) {
		_coherentcount++;
		return coherentPt;
	}*/
	return approxPt;
}

/*double LayerTextureSynthesizer::BestApproximateMatch(KDTree &kdtree, const Vector<Vec2i> &treeCoordinates, const PixelLayerSet &layers,
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
}*/

PCA<double> LayerTextureSynthesizer::ComputePCA(const AppParameters &parameters, const PixelLayerSet &layers, const NeighborhoodGenerator &generator, Vector<int> order, int iteration)
{
	const UINT neighborhoodCount = 32000;
	const UINT dimension = generator.Dimension();
	int nRadius = generator.NeighborhoodRadius();
	PCA<double> pca;

	String cache = "../TexSynCache/layersynth/" + parameters.texsyn_exemplar + "_";
	String cond = "";
	for (int i = 0; i < iteration; i++)
		cond += "-" + String(order[i]);
	String filename = cache + String("pca_nr-") + String(nRadius) + String("_layer-") +  String(order[iteration]) + String("cond") + cond + String(".txt");
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
	return pca;
}

KDTree LayerTextureSynthesizer::ConstructKDTree(PCA<double> &pca, const PixelLayerSet &layers, const NeighborhoodGenerator &generator, Vector<int> order, int iteration, Vector<Vec2i> &treeCoordinates)
{
	const UINT dimension = generator.Dimension();
	const UINT neighbors = 5;
	double *neighborhood = new double[dimension];
	KDTree kdtree;

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

	return kdtree;
}

void LayerTextureSynthesizer::Write(const PixelLayerSet &layers, PixelLayerSet &synth, const Vector< Grid<Vec2i> > &coordinateset, int width, int height, int pad, String cond)
{
	int idx = synth.Length() - 1;
	UINT exemplarwidth = layers.First().Width();
	UINT exemplarheight = layers.First().Height();
	Bitmap coordimage(height, width);
	for (int row = 0; row < height; row++) { // coordinates
		for (int col = 0; col < width; col++)
			coordimage[row][col] = RGBColor(Vec3f((float)coordinateset[idx](row+pad,col+pad).y/exemplarheight, (float)coordinateset[idx](row+pad,col+pad).x/exemplarwidth, 0));
	}
	coordimage.SavePNG(_outputdir + String("coord_") + String(idx) + cond + String(".png"));
	synth[idx].SavePNG(_outputdir + String("image_") + String(idx) + cond + String(".png"));
}