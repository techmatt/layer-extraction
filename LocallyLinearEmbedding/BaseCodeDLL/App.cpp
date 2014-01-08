#include "Main.h"
#include "segment-image.h"

const bool usingRecolorizer = false;

void App::Init()
{
	/*const String dir = "C:\\Code\\GoogleCode\\layer-extraction-paper\\trunk\\figs\\teaser\\";
	Utility::MakePaletteOverlay(Bitmap::Load(dir + "landscapePaletteA.png")).SavePNG(dir + "landscapePaletteA.png");
	Utility::MakePaletteOverlay(Bitmap::Load(dir + "landscapePaletteB.png")).SavePNG(dir + "landscapePaletteB.png");
	Utility::MakePaletteOverlay(Bitmap::Load(dir + "landscapePaletteC.png")).SavePNG(dir + "landscapePaletteC.png");
	AllocConsole();
	_parameters.Init("../Parameters.txt");

	Bitmap bmp;
	bmp.LoadPNG("../Data/" + _parameters.imageFile);

	_image = bmp;

	_extractor.Init(_parameters, bmp);

	LayerSet layers;
	if(_parameters.useKMeansPalette)
	{
	KMeansClustering<Vec3f, Vec3fKMeansMetric> clustering;
	Vector<Vec3f> bmpColors;

	for(UINT y = 0; y < bmp.Height(); y++) for(UINT x = 0; x < bmp.Width(); x++) bmpColors.PushEnd(Vec3f(bmp[y][x]));
	bmpColors.Randomize();

	clustering.Cluster(bmpColors, _parameters.KMeansPaletteSize, 10000, true, 0.000001);

	Vector<Vec3f> palette(_parameters.KMeansPaletteSize);
	for(int i = 0; i < _parameters.KMeansPaletteSize; i++) palette[i] = clustering.ClusterCenter(i);

	//palette.Sort([](const Vec3f &a, const Vec3f &b){ return a.LengthSq() < b.LengthSq(); });
	palette.Sort([](const Vec3f &a, const Vec3f &b){ return a.y < b.y; });

	_extractor.InitLayersFromPalette(_parameters, bmp, palette, layers);
	}
	else
	{
	Bitmap mask;
	mask.LoadPNG("../Data/" + _parameters.maskFile);
	Vector<PixelConstraint> pixelColors;
	Utility::AddMaskConstraints(bmp, mask, pixelColors);
	_extractor.InitLayersFromPixelConstraints(_parameters, bmp, pixelColors, layers);
	}

	_extractor.ExtractLayers(_parameters, bmp, layers);
	_extractor.AddNegativeConstraints(_parameters, bmp, layers);
	_extractor.ExtractLayers(_parameters, bmp, layers);
	_extractor.AddNegativeConstraints(_parameters, bmp, layers);
	_extractor.ExtractLayers(_parameters, bmp, layers);
	_extractor.AddNegativeConstraints(_parameters, bmp, layers);
	_extractor.ExtractLayers(_parameters, bmp, layers);
	_extractor.AddNegativeConstraints(_parameters, bmp, layers);
	_extractor.ExtractLayers(_parameters, bmp, layers);

	layers.Dump("../Results/Layers.txt", _extractor.SuperpixelColors());

	_extractor.TestLayerRecoloring(bmp, layers);
	_layers = layers;*/
}

void App::Recolorize()
{
	Console::WriteLine("Recolorizing...");
	Bitmap bmp, mask;
	bmp.LoadPNG("../Data/" + _parameters.imageFile);
	mask.LoadPNG("../Data/" + _parameters.maskFile);

	Recolorizer recolorizer;
	recolorizer.Init(_parameters, bmp);

	Vector<PixelConstraint> pixelConstraints;
	Utility::AddMaskConstraints(bmp, mask, pixelConstraints);
	Bitmap result = recolorizer.Recolor(_parameters, bmp, pixelConstraints, 0.001, 0.6);
	result.SavePNG("../Results/result.png");
}

BCLayers* App::ExtractLayers(const BCBitmapInfo &bcbmp, const Vector<Vec3f> &palette, const String &constraints, const bool autoCorrect, const String& imageFile)
{
	AllocConsole();

	Bitmap bmp;
	bmp.Allocate(bcbmp.width, bcbmp.height);
	for (UINT x=0; x<bcbmp.width; x++)
	{
		for (UINT y=0; y<bcbmp.height; y++)
		{
			int idx = 3*(y*bcbmp.width+x);
			bmp[y][x] = RGBColor(bcbmp.colorData[idx], bcbmp.colorData[idx+1], bcbmp.colorData[idx+2]);
		}
	}
	Vector<PixelLayer> pixellayers = ExtractLayers(bmp, palette, constraints, autoCorrect);

	CacheLayers(palette, pixellayers, imageFile);

	BCLayers *result = PixelLayersToBCLayers(pixellayers);

	return result;
}

void App::CacheLayers(const Vector<Vec3f> &palette, const Vector<PixelLayer> &pixellayers, const String& imageFile)
{
	Vector<String> parts;
	if (imageFile.Contains("/"))
		parts = imageFile.Partition("/");
	else
		parts = imageFile.Partition("\\");
	parts = parts[parts.Length()-1].Partition(".");
	String cachedir = "../VideoCache/";
	String cache = cachedir + parts[0] + ".dat";
	Utility::MakeDirectory(cachedir);

	OutputDataStream fileBinary;

	UINT frameCount = 1;
	fileBinary << palette.Length() << frameCount;

	for(UINT layerIndex = 0; layerIndex < palette.Length(); layerIndex++)
	{
		const auto &curLayer = pixellayers[layerIndex];
		fileBinary.WriteData(curLayer.color);
		fileBinary << curLayer.pixelWeights;
	}

	fileBinary.SaveToCompressedFile(cache);
}

PixelLayerSet App::ExtractLayers(const Bitmap &bmp, const Vector<Vec3f> &palette, const String &constraints, const bool autoCorrect)
{
	_parameters.Init("../Parameters.txt");

	if(usingRecolorizer)
	{
		Recolorize();
	}

	_image = bmp;

	_extractor.Init(_parameters, bmp);

	LayerSet layers;

	if(constraints.Length() == 0)
	{
		_extractor.InitLayersFromPalette(_parameters, bmp, palette, layers);
	}
	else
	{
		Vector<PixelConstraint> targetPixelColors;
		Vector<String> parts = constraints.Partition("|");
		for(UINT i = 0; i < parts.Length(); i++)
		{
			Vector<String> pixels = parts[i].Partition(";");
			for(String s : pixels)
			{
				PixelConstraint p;
				p.coord = Vec2i(s.Partition(",")[0].ConvertToInteger(), s.Partition(",")[1].ConvertToInteger());
				p.targetColor = palette[i];
				targetPixelColors.PushEnd(p);
			}
		}
		_extractor.InitLayersFromPixelConstraints(_parameters, bmp, targetPixelColors, layers);
	}

	//add preferences
	_extractor.AddLayerPreferenceConstraints(_parameters, bmp, layers);
	_extractor.AddMidpointConstraints(_parameters, bmp, layers);

	if (autoCorrect)
	{
		bool changed = true;
		int iterations = 0;
		while (changed && iterations < 10)
		{	
			_extractor.ExtractLayers(_parameters, bmp, layers);
			changed = _extractor.CorrectLayerSet(_parameters, bmp, layers);
			iterations++;
		}
		Console::WriteLine("Done correcting");
	}

	UINT suppressionTrials = 4;
	for(UINT negativeSuppressionIndex = 0; negativeSuppressionIndex < suppressionTrials; negativeSuppressionIndex++)
	{
		_extractor.ExtractLayers(_parameters, bmp, layers);
		_extractor.AddNegativeConstraints(_parameters, bmp, layers);
		if (_parameters.useMatlab)
			break;
	}


	layers.Dump("../Results/Layers.txt", _extractor.SuperpixelColors());

	_layers = layers;

	Vector<PixelLayer> pixellayers = _extractor.GetPixelLayers(_image, _layers);

	return pixellayers;
}

void App::ExtractVideoLayers()
{
	_parameters.Init("../Parameters.txt");

	Video video;

	const UINT frameCount = 23;
	for (UINT frameIndex = 0; frameIndex < frameCount; frameIndex++)
	{
		Bitmap bmp;
		//bmp.LoadPNG("../Data/softboy/softboy_intro_1" + String::ZeroPad(frameIndex, 3) + ".png");
		//bmp.LoadPNG("../Data/bigbuckbunny/bigbuckbunny_" + String::ZeroPad(frameIndex, 4) + ".png");
		bmp.LoadPNG("../Data/sintel-5/sintel-5_" + String::ZeroPad(frameIndex, 2) + ".png");
		video.frames.PushEnd(bmp);
	}

	LayerSet layers;
	LayerExtractorVideo videoExtractor;
	videoExtractor.Init(_parameters, video);

	videoExtractor.InitLayersFromPalette(_parameters, video, video.ComputePaletteKMeans(6), layers);
	//videoExtractor.InitLayersFromPalette(_parameters, video, video.ComputeFrame0Palette("../Data/sintel-5/sintel-5_00_palette.png"), layers);
	//videoExtractor.InitLayersFromPalette(_parameters, video, video.ComputeFrame0Palette("../Data/bigbuckbunny/bigbuckbunny_0000_palette.png"), layers);

	for(UINT negativeSuppressionIndex = 0; negativeSuppressionIndex < 4; negativeSuppressionIndex++)
	{
		videoExtractor.ExtractLayers(_parameters, video, layers);
		videoExtractor.AddNegativeConstraints(_parameters, video, layers);
	}

	//layers.Dump("../Results/Layers.txt", videoExtractor.SuperpixelColors());

	Utility::MakeDirectory("../Results/VideoLayers/");

	for(UINT layerIndex = 0; layerIndex < layers.layers.Length(); layerIndex++)
	{
		String layerDirectory = "../Results/VideoLayers/Layer" + String(layerIndex); 
		Utility::MakeDirectory(layerDirectory);
		for(UINT frameIndex = 0; frameIndex < video.frames.Length(); frameIndex++)
		{
			Bitmap bmp = videoExtractor.VisualizeLayer(_parameters, video, frameIndex, layerIndex, layers);
			bmp.SavePNG(layerDirectory + "/f" + String::ZeroPad(frameIndex, 4) + ".png");
		}
	}

	Console::WriteLine("Done extracting video layers");
}

BCLayers* App::PixelLayersToBCLayers(const PixelLayerSet &pixellayers)
{
	BCLayers *result = new BCLayers();
	result->layers = new BCLayerInfo[pixellayers.Length()];
	result->numLayers = pixellayers.Length();

	UINT width = pixellayers.First().pixelWeights.Cols();
	UINT height = pixellayers.First().pixelWeights.Rows();

	for (UINT i=0; i<pixellayers.Length(); i++)
	{
		result->layers[i].d0 = pixellayers[i].color[0];
		result->layers[i].d1 = pixellayers[i].color[1];
		result->layers[i].d2 = pixellayers[i].color[2];
		result->layers[i].width = width;
		result->layers[i].height = height;

		result->layers[i].weights = new double[width*height];
		memcpy(result->layers[i].weights, pixellayers[i].pixelWeights.CArray(), width*height*sizeof(double));
	}
	return result;
}

BCLayers* App::SynthesizeLayers()
{
	_parameters.Init("../Parameters.txt");

	//setup the layers
	int reducedDimension = _parameters.reducedDimension;
	UINT iters = 10;
	int neighborhoodSize = _parameters.neighborhoodSize;

	String layerDir = "../Layers/";
	String dataDir = "../SynthesisData/";
	PixelLayerSet target;
	for (UINT i=0; i<_parameters.targetLayers.Length(); i++)
		target.PushEnd(PixelLayer(layerDir+_parameters.targetLayers[i]));

	PixelLayerSet reference;
	for (UINT i=0; i<_parameters.refLayers.Length(); i++)
		reference.PushEnd(PixelLayer(layerDir+_parameters.refLayers[i]));

	int depth = _parameters.pyramidDepth;
	GaussianPyramid targetPyramid(target, depth);
	GaussianPyramid referencePyramid(reference, depth);


	PersistentAssert(target.Length()==reference.Length(), "Target layer count and reference layer count don't match");

	//let's standardize the reference layers so that the mean and variance value match the target
	//(like in image analogies)
	/*for (UINT i=0; i<target.Length(); i++)
	{
	double stdT = sqrt(target[i].WeightVariance());
	double stdR = sqrt(reference[i].WeightVariance());
	double meanT = target[i].WeightMean();
	double meanR = reference[i].WeightMean();

	for (UINT r=0; r<reference[i].pixelWeights.Rows(); r++)
	for (UINT c=0; c<reference[i].pixelWeights.Cols(); c++)
	reference[i].pixelWeights(r,c) = stdT*(reference[i].pixelWeights(r,c)-meanR)/stdR + meanT; 
	}*/


	//output the target and reference layers
	for(UINT i=0; i<target.Length(); i++)
	{
		target[i].SavePNG("target"+String(i)+".png", false);
		reference[i].SavePNG("reference"+String(i)+".png", false);
	}


	Grid<double> updateSchedule(iters, target.Length(), 0);

	for (UINT i=0; i<iters; i++)
	{
		/*if (i % 2 == 0)
		updateSchedule(i,0) = 1;
		else
		{
		updateSchedule(i,1) = 0.5;
		}*/
		updateSchedule.SetRow(i, _parameters.updateSchedule);
	}

	Bitmap orig, mask;
	orig.LoadPNG(dataDir+_parameters.targetImageFile);
	mask.LoadPNG(dataDir+_parameters.targetMaskFile);

	Vector<Vec2i> pixels;
	UINT width = target.First().pixelWeights.Cols();
	UINT height = target.First().pixelWeights.Rows();
	for (UINT x=0; x<width; x++)
		for (UINT y=0; y<height; y++)
			if (orig[y][x] != mask[y][x])
				pixels.PushEnd(Vec2i(x,y));

	NeighborhoodGenerator generator(neighborhoodSize, target.Length(), depth);
	LayerSynthesis synthesizer;
	synthesizer.Init(referencePyramid, generator, reducedDimension);
	PixelLayerSet synth = synthesizer.Synthesize(_parameters, referencePyramid, targetPyramid, pixels, updateSchedule, generator);


	//index back into original
	PixelLayerSet result;
	for (UINT i=0; i<_parameters.allTargetLayers.Length(); i++)
	{
		int index = _parameters.targetLayers.FindFirstIndex(_parameters.allTargetLayers[i]);
		if (index >= 0)
			result.PushEnd(synth[index]);
		else
			result.PushEnd(PixelLayer(layerDir+_parameters.allTargetLayers[i]));
	}


	return PixelLayersToBCLayers(result);

}



void App::SynthesizeTextureByLayers(const String &parameterFilename)
{
	if(parameterFilename.Length() > 0) _parameters.Init(parameterFilename);
	else _parameters.Init("../Parameters.txt");

	int reducedDimension = _parameters.texsyn_pcadim;
	int neighborhoodSize = _parameters.texsyn_neighbourhoodsize; // radius
	int outputwidth = _parameters.texsyn_outputwidth;
	int outputheight = _parameters.texsyn_outputheight;
	int nlevels = _parameters.texsyn_nlevels;

	// read in exemplar image
	String exemplar_name = "../TextureSynthesisExemplars/" + _parameters.texsyn_exemplar;
	Console::WriteLine("Texture Synthesis By Layers: " + exemplar_name);
	Bitmap rgbimg;
	rgbimg.LoadPNG(exemplar_name);
	//debugging
	String outdir = "texsyn-out/LAYER/";
	rgbimg.SavePNG(outdir + String("input.png"));

	// input layers for synthesis
	PixelLayerSet input, rgblayers;

	// split colour image to rgb pixel layers
	PixelLayer red, green, blue;
	red.color = Vec3f(1, 0, 0);
	green.color = Vec3f(0, 1, 0);
	blue.color = Vec3f(0, 0, 1);
	red.pixelWeights.Allocate(rgbimg.Height(), rgbimg.Width());
	green.pixelWeights.Allocate(rgbimg.Height(), rgbimg.Width());
	blue.pixelWeights.Allocate(rgbimg.Height(), rgbimg.Width());
	for (UINT y = 0; y < rgbimg.Height(); y++) {
		for (UINT x = 0; x < rgbimg.Width(); x++) {
			Vec3f colour = Vec3f(rgbimg[y][x]);
			red.pixelWeights(y,x) = colour[0];
			green.pixelWeights(y,x) = colour[1];
			blue.pixelWeights(y,x) = colour[2];
		}
	}
	rgblayers.PushEnd(red);
	rgblayers.PushEnd(green);
	rgblayers.PushEnd(blue);

	// use layers from k-means colour selection
	if (_parameters.texsyn_uselayers) {
		// check if layers are cached
		String cache = String("../TexSynCache/") + _parameters.texsyn_exemplar + String("_");
		String palettecache = cache + String("kmeans-palette_k-") + String(_parameters.texsyn_klayers) + String(".txt");
		if (Utility::FileExists(palettecache)) { 
			// load from file
			Vector<Vec3f> palette;
			Vector<String> lines = Utility::GetFileLines(palettecache);
			for (int i = 0; i < _parameters.texsyn_klayers; i++) {
				Vector<String> colourFields = lines[i].Partition('\t');
				palette.PushEnd(Vec3f(colourFields[0].ConvertToFloat(), colourFields[1].ConvertToFloat(), colourFields[2].ConvertToFloat()));
			}

			int idx = input.Length();
			for (int i = 0; i < _parameters.texsyn_klayers; i++) {
				String layerfile = cache + String("layers_k-") + String(_parameters.texsyn_klayers) + String("_l-") + String(i) + String(".txt");
				input.PushEnd(PixelLayer(layerfile));
				input[idx+i].SavePNG(outdir + String("input-layer-") + String(i) + String(".png"));
			}
		}
		else { // generate palette & layers
			LayerSet layers;
			KMeansClustering<Vec3f, Vec3fKMeansMetric> clustering;
			Vector<Vec3f> bmpColors;

			for(UINT y = 0; y < rgbimg.Height(); y++) for(UINT x = 0; x < rgbimg.Width(); x++) bmpColors.PushEnd(Vec3f(rgbimg[y][x]));
			bmpColors.Randomize();

			clustering.Cluster(bmpColors, _parameters.texsyn_klayers, 10000, true, 0.000001);

			Vector<Vec3f> palette(_parameters.texsyn_klayers);
			for(int i = 0; i < _parameters.texsyn_klayers; i++) palette[i] = clustering.ClusterCenter(i);
			palette.Sort([](const Vec3f &a, const Vec3f &b){ return a.y < b.y; });

			_extractor.Init(_parameters, rgbimg);
			_extractor.InitLayersFromPalette(_parameters, rgbimg, palette, layers);
			_extractor.ExtractLayers(_parameters, rgbimg, layers);
			for (int i = 0; i < 4; i++) {
				_extractor.AddNegativeConstraints(_parameters, rgbimg, layers);
				_extractor.ExtractLayers(_parameters, rgbimg, layers);
			}
			PixelLayerSet p = _extractor.GetPixelLayers(rgbimg, layers);
			input.Append(p);

			// cache palette
			ofstream File(palettecache.CString());
			PersistentAssert(!File.fail(), "Failed to open file");
			for (int i = 0; i < _parameters.texsyn_klayers; i++)
				File << palette[i].TabSeparatedString() << endl;
			File.close();
			// cache layers
			for (int i = 0; i < _parameters.texsyn_klayers; i++) {
				String layerfile = cache + String("layers_k-") + String(_parameters.texsyn_klayers) + String("_l-") + String(i) + String(".txt");
				p[i].WriteToFile(layerfile);
				p[i].SavePNG(outdir + String("input-layer-") + String(i) + String(".png"));
			}
		}
	}

	Vector<int> order(input.Length());
	for (UINT i = 0; i < input.Length(); i++)
		order[i] = i;//(i+1) % input.Length();

	NeighborhoodGenerator generator(neighborhoodSize, input.Length(), 1);
	// synthesize
	LayerTextureSynthesizer synthesizer;
	synthesizer.Synthesize(input, rgblayers, _parameters, generator, order);
}

bool App::ReadLayersFromCache(const String& cache, PixelLayerSet &layers, Vector<Vec3f>& palette, const Vector<PixelConstraint>& constraints)
{
	unsigned int K = layers.Length();
	String cachefile = cache;
	String layercache = "layers_";
	if (constraints.Length() == 0) {
		cachefile += "kmeans-palette_k-" + String(K) + ".txt";
		layercache += "k-" + String(K);
	}
	else {
		K = constraints.Length();
		cachefile += "user-" + String(K) + ".txt";
		layercache += "user-" + String(K);
	}
	if (Utility::FileExists(cachefile)) { 
		// load from file
		Vector<String> lines = Utility::GetFileLines(cachefile);
		for (unsigned int i = 0; i < K; i++) {
			Vector<String> colourFields = lines[i].Partition('\t');
			palette[i] = Vec3f(colourFields[0].ConvertToFloat(), colourFields[1].ConvertToFloat(), colourFields[2].ConvertToFloat());
		}
		InputDataStream stream;
		stream.LoadFromFile(cache + layercache + ".dat");
		for (unsigned int i = 0; i < K; i++) {
			layers[i].color = palette[i];
			stream >> layers[i].pixelWeights;
		}
		Console::WriteLine("read layers from cache");
		return true;
	}
	return false;
}

void App::ComputeLayers(const String& cache, const Bitmap& image, PixelLayerSet &layers, Vector<Vec3f>& palette, const Vector<PixelConstraint>& constraints)
{
	unsigned int K = layers.Length();
	String cachefile = cache;
	String layercache = "layers_";
	if (constraints.Length() == 0) {
		cachefile += "kmeans-palette_k-" + String(K) + ".txt";
		layercache += "k-" + String(K);
	}
	else {
		cachefile += "user-" + String(constraints.Length()) + ".txt";
		layercache += "user-" + String(K);
	}

	// generate palette & layers
	LayerSet superpixellayers;
	_extractor.Init(_parameters, image);
	if (constraints.Length() == 0) { // kmeans palette
		KMeansClustering<Vec3f, Vec3fKMeansMetric> clustering;
		Vector<Vec3f> bmpColors;

		for(UINT y = 0; y < image.Height(); y++) for(UINT x = 0; x < image.Width(); x++) bmpColors.PushEnd(Vec3f(image[y][x]));
		bmpColors.Randomize();

		clustering.Cluster(bmpColors, K, 10000, true, 0.000001);

		for(unsigned int i = 0; i < K; i++) palette[i] = clustering.ClusterCenter(i);
		palette.Sort([](const Vec3f &a, const Vec3f &b){ return a.y < b.y; });

		_extractor.InitLayersFromPalette(_parameters, image, palette, superpixellayers);
	}
	else {
		K = constraints.Length();
		for (unsigned int i = 0; i < constraints.Length(); i++) {
			palette[i] = constraints[i].targetColor;
		}
		_extractor.InitLayersFromPixelConstraints(_parameters, image, constraints, superpixellayers);
	}
	// cache palette
	ofstream File(cachefile.CString());
	PersistentAssert(!File.fail(), "Failed to open file");
	for (unsigned int i = 0; i < K; i++)
		File << palette[i].TabSeparatedString() << endl;
	File.close();

	// layers
	_extractor.ExtractLayers(_parameters, image, superpixellayers);
	for (int i = 0; i < 4; i++) {
		_extractor.AddNegativeConstraints(_parameters, image, superpixellayers);
		_extractor.ExtractLayers(_parameters, image, superpixellayers);
	}
	layers = _extractor.GetPixelLayers(image, superpixellayers);

	// cache layers
	OutputDataStream stream;
	for (unsigned int i = 0; i < K; i++) {
		stream << layers[i].pixelWeights;
	}
	stream.SaveToFile(cache + layercache + ".dat");
}

void App::FilterLayers(const String &parameterFilename)
{
	if(parameterFilename.Length() > 0) _parameters.Init(parameterFilename);
	else _parameters.Init("../Parameters.txt");

	int klayers = 5;

	String imagename = "star-cropped.png";
	String palettename = "star-cropped-palette.png";
	String imagelocation = "../Data/";
	String cachedir = "../TexSynCache/";
	String outdir = "./filters/";

	// read in image
	Bitmap image;
	image.LoadPNG(imagelocation + imagename);
	image.SavePNG(outdir + "filter-input.png");

	// palette
	Vector<PixelConstraint> constraints;
	bool userpalette = false;
	if (Utility::FileExists(imagelocation + palettename)) {
		userpalette = true;
		constraints = _extractor.ComputePalette(imagelocation + palettename, image);
		klayers = (int) constraints.Length();
	}

	// check if layers are cached
	String cache = cachedir + imagename + "_";
	PixelLayerSet layers(klayers);
	Vector<Vec3f> palette(klayers);
	if (!ReadLayersFromCache(cache, layers, palette, constraints))
		ComputeLayers(cache, image, layers, palette, constraints);
	for (unsigned int i = 0; i < layers.Length(); i++) {
		layers[i].SavePNG(outdir + "layers_k-" + String(layers.Length()) + "_l-" + String(i) + ".png");
	}

	Bitmap result(image.Width(), image.Height());
	// re-compose
	for (UINT y = 0; y < image.Height(); y++) {
		for (UINT x = 0; x < image.Width(); x++) {
			Vec3f v(0,0,0);
			for (int k = 0; k < (int)layers.Length(); k++)
				v += layers[k].color * (float)layers[k].pixelWeights(y,x);
			result[y][x] = RGBColor(v);
		}
	}
	result.SavePNG(outdir + "filter-reconstruction.png");

	Vector<int> editlayers;
	for (int i = 0; i < klayers; i++)
		editlayers.PushEnd(i);
	const double sigma = 25.0;
	const float scale = 1.0;
	const int grid_radius = 15;

	// filter 
	PixelLayerSet filteredlayers(layers.Length());
	for (unsigned int i = 0; i < editlayers.Length(); i++) {
		filteredlayers[editlayers[i]] = PixelLayer(layers[editlayers[i]]);

		//filteredlayers[editlayers[i]].pixelWeights.GaussianBlur(sigma, scale);
		//filteredlayers[editlayers[i]].pixelWeights = filteredlayers[editlayers[i]].pixelWeights * 0.2;
		//filteredlayers[editlayers[i]].pixelWeights += layers[editlayers[i]].pixelWeights;
		//filteredlayers[editlayers[i]].pixelWeights.MedianFilter(grid_radius);
		//filteredlayers[editlayers[i]].pixelWeights.BilateralFilter(grid_radius, 0.1);
		//filteredlayers[editlayers[i]].pixelWeights.MotionBlur(Vec2f(1, 1), 41, 1.0);
		/*filteredlayers[editlayers[i]].pixelWeights.Laplacian();
		filteredlayers[editlayers[i]].pixelWeights *= 6.0;
		filteredlayers[editlayers[i]].pixelWeights.GaussianBlur(2, scale);*/
		filteredlayers[editlayers[i]].pixelWeights.Emboss();
		filteredlayers[editlayers[i]].pixelWeights += layers[editlayers[i]].pixelWeights;
		//filteredlayers[editlayers[i]].pixelWeights.Sharpen();
		//filteredlayers[editlayers[i]].pixelWeights.Erode(1.0);

		//Console::WriteLine("[ layer " + String(editlayers[i]) + " ] gaussian blur, sigma = " + String(sigma) + " , scale = " + String(scale));
		Console::WriteLine("[ layer " + String(editlayers[i]) + " ] emboss");

		filteredlayers[editlayers[i]].SavePNG(outdir + "layers_k-" + String(klayers) + "_l-" + String(editlayers[i]) + "-filtered.png");
		// re-compose
		for (UINT y = 0; y < image.Height(); y++) {
			for (UINT x = 0; x < image.Width(); x++) {
				Vec3f v(0,0,0);
				for (int k = 0; k < (int)layers.Length(); k++) {
					if (k != editlayers[i])
						v += layers[k].color * (float)layers[k].pixelWeights(y,x);
					else
						v += filteredlayers[k].color * (float)filteredlayers[k].pixelWeights(y,x);
				}
				result[y][x] = RGBColor(v);
			}
		}
		result.SavePNG(outdir + "filter-output-" + String(editlayers[i]) + ".png");
	}

	/*
	for (UINT y = 0; y < image.Height(); y++) {
	for (UINT x = 0; x < image.Width(); x++) {
	Vec3f v(0,0,0);
	for (int k = 0; k < 5; k++) 
	v += layers[k].color * (float)layers[k].pixelWeights(y,x);
	for (int k = 5; k < 7; k++) 
	v += filteredlayers[k].color * (float)filteredlayers[k].pixelWeights(y,x);
	for (int k = 7; k < (int)layers.Length(); k++) 
	v += layers[k].color * (float)layers[k].pixelWeights(y,x);
	result[y][x] = RGBColor(v);
	}
	}
	result.SavePNG(outdir + "filter-output-56.png");*/

	Console::WriteLine("Done!");
}

void App::DeleteLayer(const String &parameterFilename)
{
	if(parameterFilename.Length() > 0) _parameters.Init(parameterFilename);
	else _parameters.Init("../Parameters.txt");

	const int klayers = 5;
	const int idx = 4; // layer to delete
	bool del = true;
	String imagename = "water.png";
	String imagelocation = "../TextureSynthesisExemplars/";
	String cachedir = "../TexSynCache/";
	String outdir = "./";

	// read in image
	Bitmap image;
	image.LoadPNG(imagelocation + imagename);

	image.SavePNG(outdir + "input.png");

	// check if layers are cached
	PixelLayerSet layers;
	Vector<Vec3f> palette(klayers);
	String cache = cachedir + imagename + "_";
	String palettecache = cache + "kmeans-palette_k-" + String(klayers) + ".txt";
	if (Utility::FileExists(palettecache)) { 
		// load from file
		Vector<String> lines = Utility::GetFileLines(palettecache);
		for (int i = 0; i < klayers; i++) {
			Vector<String> colourFields = lines[i].Partition('\t');
			palette[i] = Vec3f(colourFields[0].ConvertToFloat(), colourFields[1].ConvertToFloat(), colourFields[2].ConvertToFloat());
		}
		for (int i = 0; i < klayers; i++) {
			String layerfile = cache + "layers_k-" + String(klayers) + "_l-" + String(i) + ".txt";
			layers.PushEnd(PixelLayer(layerfile));
			layers[i].SavePNG(outdir + "layers_k-" + String(klayers) + "_l-" + String(i) + ".png");
		}
		Console::WriteLine("read layers from cache");
	}
	else { // generate palette & layers
		LayerSet superpixellayers;
		KMeansClustering<Vec3f, Vec3fKMeansMetric> clustering;
		Vector<Vec3f> bmpColors;

		for(UINT y = 0; y < image.Height(); y++) for(UINT x = 0; x < image.Width(); x++) bmpColors.PushEnd(Vec3f(image[y][x]));
		bmpColors.Randomize();

		clustering.Cluster(bmpColors, klayers, 10000, true, 0.000001);

		for(int i = 0; i < klayers; i++) palette[i] = clustering.ClusterCenter(i);
		palette.Sort([](const Vec3f &a, const Vec3f &b){ return a.y < b.y; });

		_extractor.Init(_parameters, image);
		_extractor.InitLayersFromPalette(_parameters, image, palette, superpixellayers);
		_extractor.ExtractLayers(_parameters, image, superpixellayers);
		for (int i = 0; i < 4; i++) {
			_extractor.AddNegativeConstraints(_parameters, image, superpixellayers);
			_extractor.ExtractLayers(_parameters, image, superpixellayers);
		}
		layers = _extractor.GetPixelLayers(image, superpixellayers);

		// cache palette
		ofstream File(palettecache.CString());
		PersistentAssert(!File.fail(), "Failed to open file");
		for (int i = 0; i < klayers; i++)
			File << palette[i].TabSeparatedString() << endl;
		File.close();
		// cache layers
		for (int i = 0; i < klayers; i++) {
			String layerfile = cache + "layers_k-" + String(klayers) + "_l-" + String(i) + ".txt";
			layers[i].WriteToFile(layerfile);
			layers[i].SavePNG(outdir + "layers_k-" + String(klayers) + "_l-" + String(i) + ".png");
		}
	}

	// re-compose (debugging)
	Bitmap debug(image.Width(), image.Height());
	for (UINT y = 0; y < image.Height(); y++) {
		for (UINT x = 0; x < image.Width(); x++) {
			Vec3f v(0,0,0);
			for (int k = 0; k < (int)layers.Length(); k++)
				v += layers[k].color * (float)layers[k].pixelWeights(y,x);
			debug[y][x] = RGBColor(v);
		}
	}
	debug.SavePNG(outdir + "output-debug.png");

	if (del) {
		// erase layer
		Bitmap output(image.Width(), image.Height());
		for (UINT y = 0; y < image.Height(); y++) {
			for (UINT x = 0; x < image.Width(); x++) {
				Vec3f v(0,0,0);
				for (int k = 0; k < (int)layers.Length(); k++)
					v += layers[k].color * (float)layers[k].pixelWeights(y,x);
				v -= layers[idx].color * (float)layers[idx].pixelWeights(y,x);
				output[y][x] = RGBColor(v);
			}
		}
		output.SavePNG(outdir + "output-erased.png");

		// scale to 1 approach
		float scale;
		for (UINT y = 0; y < image.Height(); y++) {
			for (UINT x = 0; x < image.Width(); x++) {
				Vec3f v(0,0,0);
				scale = 0;
				for (int k = 0; k < klayers; k++) { 
					v += layers[k].color * (float)layers[k].pixelWeights(y,x);
					scale += (float) layers[k].pixelWeights(y,x);
				}
				v -= layers[idx].color * (float)layers[idx].pixelWeights(y,x);
				scale -= (float) layers[idx].pixelWeights(y,x);
				if (scale < FLT_EPSILON) scale = 1;
				v /= scale;
				output[y][x] = RGBColor(v);
			}
		}
		output.SavePNG(outdir + "output-scaled.png");

		PixelLayerSet nlayers;
		Vector<Vec3f> npalette;
		for (int k = 0; k < klayers; k++) {
			if (k != idx)
				npalette.PushEnd(palette[k]);
		}
		// check if layers are cached
		bool cached = true;
		for (int k = 0; k < klayers-1; k++) {
			String layercache = cache + "nlayer-" + String(k) + "-of-" + String(klayers-1) + "_colour-" + 
				String(npalette[k][0]) + "-" + String(npalette[k][1]) + "-" + String(npalette[k][2]) + ".txt";
			if (Utility::FileExists(layercache)) {
				nlayers.PushEnd(PixelLayer(layercache));
				nlayers[k].SavePNG(outdir + "nlayer-" + String(k) + "-of-" + String(klayers-1) + "_colour-" + 
					String(npalette[k][0]) + "-" + String(npalette[k][1]) + "-" + String(npalette[k][2]) + ".png");
			}
			else {
				cached = false;
				break;
			}
		}
		if (cached)
			Console::WriteLine("read layers from cache");
		else {
			nlayers.FreeMemory();
			LayerSet superpixellayers;

			_extractor.Init(_parameters, image);
			_extractor.InitLayersFromPalette(_parameters, image, npalette, superpixellayers);
			_extractor.ExtractLayers(_parameters, image, superpixellayers);
			for (int i = 0; i < 4; i++) {
				_extractor.AddNegativeConstraints(_parameters, image, superpixellayers);
				_extractor.ExtractLayers(_parameters, image, superpixellayers);
			}
			nlayers = _extractor.GetPixelLayers(image, superpixellayers);

			// cache layers
			for (int k = 0; k < klayers-1; k++) {
				String layercache = cache + "nlayer-" + String(k) + "-of-" + String(klayers-1) + "_colour-" + 
					String(npalette[k][0]) + "-" + String(npalette[k][1]) + "-" + String(npalette[k][2]) + ".txt";
				nlayers[k].WriteToFile(layercache);
				nlayers[k].SavePNG(outdir + "nlayer-" + String(k) + "-of-" + String(klayers-1) + "_colour-" + 
					String(npalette[k][0]) + "-" + String(npalette[k][1]) + "-" + String(npalette[k][2]) + ".png");
			}
		}
		// scaling
		//Grid<int> mask(image.Height(), image.Width(), 1);
		Vector<float> scaled(klayers-1);
		int i;
		for (UINT y = 0; y < image.Height(); y++) {
			for (UINT x = 0; x < image.Width(); x++) {
				Vec3f v(0,0,0);
				i = 0;
				scale = 0;
				for (int k = 0; k < klayers; k++) {
					if (k != idx) {
						scaled[i] = (float) max(max(layers[k].pixelWeights(y,x), nlayers[i].pixelWeights(y,x)), layers[k].pixelWeights(y,x) * nlayers[i].pixelWeights(y,x));
						scale += scaled[i];
						i++;
					}
				}
				if (scale < .01) {
					scale = 1;
					//mask(y,x) = 0;
				}
				for (int k = 0; k < klayers-1; k++) 
					v += nlayers[k].color * scaled[k] / scale;
				output[y][x] = RGBColor(v);
			}
		}
		output.SavePNG(outdir + "output-scaled-2.png");
	}
	Console::WriteLine("Done!");
}
/*
void App::FillHoles(Bitmap& image, Grid<int>& mask)
{
// gaussian filter
double sigma = 1.5;
double denom = -2 * sigma * sigma;
double filter[5][5];
for (int i = -2; i <= 2; i++) {
for (int j = -2; j <= 2; j++)
filter[i+2][j+2] = exp((i*i+j*j)/denom);
}

// fill from outside in
const int on_queue = 2;
std::queue<Vec2i> queue;
for (int y = 0; y < image.Height(); y++) {
for (int x = 0; x < image.Width(); x++) {
if (mask(y,x) == 0) continue; // unknown
if (mask(y,x) == 2) continue; // on queue

// neighbors with unknown values
if (x > 0 && mask(y,x-1) == 0) {
queue.push(Vec2i(x-1,y));
mask(y,x-1) = 2;
}
if (x < image.Width()-1 && mask(y,x+1) == 0) {
queue.push(Vec2i(x+1,y));
mask(y,x+1) = 2;
}
if (y > 0 && mask(y-1,x) == 0) {
queue.push(Vec2i(x,y-1));
mask(y-1,x) = 2;
}
if (y < image.Height()-1 && mask(y+1,x) == 0) {
queue.push(Vec2i(x,y+1));
mask(y+1,x) = 2;
}
}
}
// update
Vec3f sum;
double weight;
while (!queue.empty()) {
Vec2i coord = queue.front();
queue.pop();
Assert(coord.x >= 0 && coord.y >= 0 && coord.x < image.Width() && coord.y < image.Width(),"coords out of bounds");
Assert(mask(coord.y,coord.x) == 2, "not on queue");
// filter
sum = Vec3f(0,0,0); weight = 0;
for (int i = -2; i <= 2; i++) {
for (int j = -2; j <= 2; j++) {
if (coord.x+i < 0 || coord.x+i >= image.Width()) continue;
if (coord.y+j < 0 || coord.y+j >= image.Height()) continue;
if (mask(coord.y+j,coord.x+i) == 0 || mask(coord.y+j,coord.x+i) == 2) continue;
sum += filter[i+2][j+2] * Vec3f(image[coord.y+j][coord.x+i]);
weight += filter[i+2][j+2];
}
}
if (weight > 0) image[coord.y][coord.x] = RGBColor(sum / weight);
// neighbors with unknown values
if (coord.x > 0 && mask(coord.y,coord.x-1) == 0) {
queue.push(Vec2i(coord.x-1,coord.y));
mask(coord.y,coord.x-1) = 2;
}
if (coord.x < image.Width()-1 && mask(coord.y,coord.x+1) == 0) {
queue.push(Vec2i(coord.x+1,coord.y));
mask(coord.y,coord.x+1) = 2;
}
if (coord.y > 0 && mask(coord.y-1,coord.x) == 0) {
queue.push(Vec2i(coord.x,coord.y-1));
mask(coord.y-1,coord.x) = 2;
}
if (coord.y < image.Height()-1 && mask(coord.y+1,coord.x) == 0) {
queue.push(Vec2i(coord.x,coord.y+1));
mask(coord.y+1,coord.x) = 2;
}
}
}
*/
void App::SynthesizeTexture(const String &parameterFilename)
{
	if(parameterFilename.Length() > 0) _parameters.Init(parameterFilename);
	else _parameters.Init("../Parameters.txt");

	TextureSynthesis synthesizer;

	int reducedDimension = _parameters.texsyn_pcadim;
	int neighborhoodSize = _parameters.texsyn_neighbourhoodsize; // radius
	int outputwidth = _parameters.texsyn_outputwidth;
	int outputheight = _parameters.texsyn_outputheight;
	int nlevels = _parameters.texsyn_nlevels;

	// read in exemplar image
	String exemplar_name = "../TextureSynthesisExemplars/" + _parameters.texsyn_exemplar;
	Console::WriteLine("Texture Synthesis: " + exemplar_name);
	Bitmap rgbimg;
	rgbimg.LoadPNG(exemplar_name);
	//debugging
	String outdir = synthesizer.CreateOutputDirectory(_parameters);
	if (!Utility::FileExists(outdir))
		Utility::MakeDirectory(outdir);

	// results directory
	String results_dir = "../TextureSynthesisResults/";
	if (!Utility::FileExists(results_dir)) Utility::MakeDirectory(results_dir);
	results_dir += _parameters.texsyn_exemplar + "/";
	if (!Utility::FileExists(results_dir)) {
		Utility::MakeDirectory(results_dir);
		rgbimg.SavePNG(results_dir + String("exemplar.png"));
	}

	rgbimg.SavePNG(outdir + String("input.png"));

	// input layers for synthesis
	PixelLayerSet input, rgblayers;

	// split colour image to rgb pixel layers
	PixelLayer red, green, blue;
	red.color = Vec3f(1, 0, 0);
	green.color = Vec3f(0, 1, 0);
	blue.color = Vec3f(0, 0, 1);
	red.pixelWeights.Allocate(rgbimg.Height(), rgbimg.Width());
	green.pixelWeights.Allocate(rgbimg.Height(), rgbimg.Width());
	blue.pixelWeights.Allocate(rgbimg.Height(), rgbimg.Width());
	for (UINT y = 0; y < rgbimg.Height(); y++) {
		for (UINT x = 0; x < rgbimg.Width(); x++) {
			Vec3f colour = Vec3f(rgbimg[y][x]);
			red.pixelWeights(y,x) = colour[0];
			green.pixelWeights(y,x) = colour[1];
			blue.pixelWeights(y,x) = colour[2];
		}
	}
	if (_parameters.texsyn_usergb) {
		input.PushEnd(red);
		input.PushEnd(green);
		input.PushEnd(blue);

		red.SavePNG(outdir + String("input-rgb-r.png"));
		green.SavePNG(outdir + String("input-rgb-g.png"));
		blue.SavePNG(outdir + String("input-rgb-b.png"));
	}
	rgblayers.PushEnd(red);
	rgblayers.PushEnd(green);
	rgblayers.PushEnd(blue);
	if (_parameters.texsyn_usergbdistance) {
		for (UINT i = 0; i < rgblayers.Length(); i++) {
			PixelLayer p;
			p.color = rgblayers[i].color;
			p.pixelWeights = DistanceTransform::Transform(rgblayers[i].pixelWeights);
			p.SavePNG(outdir + "input-rgb-" + String(i) + "-dist_trans.png");
			input.PushEnd(p);
		}
	}

	// use layers from k-means colour selection
	if (_parameters.texsyn_uselayers || _parameters.texsyn_usedistancetransform) {
		int idx = input.Length();
		// check if layers are cached
		PixelLayerSet layers;
		String cache = String("../TexSynCache/") + _parameters.texsyn_exemplar + String("_");
		String palettecache = cache + String("kmeans-palette_k-") + String(_parameters.texsyn_klayers) + String(".txt");
		if (Utility::FileExists(palettecache)) { 
			// load from file
			Vector<Vec3f> palette;
			Vector<String> lines = Utility::GetFileLines(palettecache);
			for (int i = 0; i < _parameters.texsyn_klayers; i++) {
				Vector<String> colourFields = lines[i].Partition('\t');
				palette.PushEnd(Vec3f(colourFields[0].ConvertToFloat(), colourFields[1].ConvertToFloat(), colourFields[2].ConvertToFloat()));
			}

			for (int i = 0; i < _parameters.texsyn_klayers; i++) {
				String layerfile = cache + String("layers_k-") + String(_parameters.texsyn_klayers) + String("_l-") + String(i) + String(".txt");
				layers.PushEnd(PixelLayer(layerfile));
				if (_parameters.texsyn_uselayers) {
					input.PushEnd(PixelLayer(layerfile));
					input[idx+i].SavePNG(outdir + String("input-layer-") + String(i) + String(".png"));
				}
			}
		}
		else { // generate palette & layers
			LayerSet superpixellayers;
			KMeansClustering<Vec3f, Vec3fKMeansMetric> clustering;
			Vector<Vec3f> bmpColors;

			for(UINT y = 0; y < rgbimg.Height(); y++) for(UINT x = 0; x < rgbimg.Width(); x++) bmpColors.PushEnd(Vec3f(rgbimg[y][x]));
			bmpColors.Randomize();

			clustering.Cluster(bmpColors, _parameters.texsyn_klayers, 10000, true, 0.000001);

			Vector<Vec3f> palette(_parameters.texsyn_klayers);
			for(int i = 0; i < _parameters.texsyn_klayers; i++) palette[i] = clustering.ClusterCenter(i);
			palette.Sort([](const Vec3f &a, const Vec3f &b){ return a.y < b.y; });

			_extractor.Init(_parameters, rgbimg);
			_extractor.InitLayersFromPalette(_parameters, rgbimg, palette, superpixellayers);
			_extractor.ExtractLayers(_parameters, rgbimg, superpixellayers);
			for (int i = 0; i < 4; i++) {
				_extractor.AddNegativeConstraints(_parameters, rgbimg, superpixellayers);
				_extractor.ExtractLayers(_parameters, rgbimg, superpixellayers);
			}
			layers = _extractor.GetPixelLayers(rgbimg, superpixellayers);
			if (_parameters.texsyn_uselayers)
				input.Append(layers);

			// cache palette
			ofstream File(palettecache.CString());
			PersistentAssert(!File.fail(), "Failed to open file");
			for (int i = 0; i < _parameters.texsyn_klayers; i++)
				File << palette[i].TabSeparatedString() << endl;
			File.close();
			// cache layers
			for (int i = 0; i < _parameters.texsyn_klayers; i++) {
				String layerfile = cache + String("layers_k-") + String(_parameters.texsyn_klayers) + String("_l-") + String(i) + String(".txt");
				layers[i].WriteToFile(layerfile);
				if (_parameters.texsyn_uselayers)
					layers[i].SavePNG(outdir + String("input-layer-") + String(i) + String(".png"));
			}
		}

		// compute distance transforms
		if (_parameters.texsyn_usedistancetransform) {
			for(UINT i = 0; i < layers.Length(); i++)
			{
				PixelLayer p;
				p.color = layers[i].color;
				p.pixelWeights = DistanceTransform::Transform(layers[i].pixelWeights);
				p.SavePNG(outdir + "input-layer-" + String(i) + "-dist_trans.png");
				input.PushEnd(p);
			}
		}
	}
	String info = "";
	if (_parameters.texsyn_usergb) info += "using rgb | ";
	if (_parameters.texsyn_uselayers) info += "using layers | ";
	if (_parameters.texsyn_usedistancetransform) info += "using distance transform (layers) | ";
	Console::WriteLine(info + String(input.Length()) + " layers");

	// build gaussian pyramid
	NeighborhoodGenerator generator(neighborhoodSize, input.Length(), 1);
	GaussianPyramid pyramid(input, nlevels);
	GaussianPyramid rgbpyr(rgblayers, nlevels); // for visualising synthesis
	// synthesize
	synthesizer.Init(_parameters, results_dir, pyramid, generator, nlevels, reducedDimension);
	synthesizer.Synthesize(rgbpyr, pyramid, _parameters, generator);
}



//Li et al Instant Propagation Video Recoloring
void App::RBFVideoRecolor()
{
_parameters.Init("../Parameters.txt");

	Video video;

	const UINT frameCount = 124;
	for (UINT frameIndex = 0; frameIndex < frameCount; frameIndex++)
	{
		Bitmap bmp;
		//bmp.LoadPNG("../Data/softboy/softboy_intro_1" + String::ZeroPad(frameIndex, 3) + ".png");
		bmp.LoadPNG("../Data/bigbuckbunny/bigbuckbunny_" + String::ZeroPad(frameIndex, 4) + ".png");
		//bmp.LoadPNG("../Data/sintel-5/sintel-5_" + String::ZeroPad(frameIndex, 2) + ".png");
		//bmp.LoadPNG("../Data/flowersBlur.png");
		video.frames.PushEnd(bmp);
	}

	
	Vector<PixelConstraint> constraints;
	Bitmap original;
	//original.LoadPNG("../Data/flowersBlur.png");
	original.LoadPNG("../Data/bigbuckbunny/bigbuckbunny_0000.png");

	Bitmap mask;
	//mask.LoadPNG("../Data/flowersBlur_denseMask.png");
	mask.LoadPNG("../Data/bigbuckbunny_mask.png");

	Utility::AddMaskConstraints(original, mask, constraints);

	Console::WriteLine("Recoloring...");

	RBFPropagation recolorer;
	Video result = recolorer.Recolor(_parameters, video, constraints);

	Console::WriteLine("Rendering frames");

	Utility::MakeDirectory("../Results/RBFResults/");

	String videoDirectory = "../Results/RBFResults/bigbuckbunny";
	Utility::MakeDirectory(videoDirectory);

	for(UINT frameIndex = 0; frameIndex < video.frames.Length(); frameIndex++)
	{
		result.frames[frameIndex].SavePNG(videoDirectory + "/f" + String::ZeroPad(frameIndex, 4) + ".png");
	}

	Console::WriteLine("Done recoloring video");

}

UINT32 App::ProcessCommand(const String &command)
{
	Vector<String> words = command.Partition(" ");
	while(words.Length() < 5) words.PushEnd("");

	if (words[0] == "SynthesizeTexture") {
		SynthesizeTexture(words[1]);
	}
	else if (words[0] == "SynthesizeTextureByLayers") {
		SynthesizeTextureByLayers(words[1]);
	}
	else if (words[0] == "DeleteLayer") {
		//DeleteLayer(words[1]);
		FilterLayers(words[1]);
	}
	else if (words[0] == "ExtractVideoLayers") {
		ExtractVideoLayers();
	}
	else if (words[0] == "RBFVideoRecolor") {
		RBFVideoRecolor();
	}
	

	return 0;
}


BCBitmapInfo* App::QueryBitmapByName(const String &s)
{
	Bitmap *resultPtr = NULL;

	if(s == "videoFrame")
	{
		if (_videocontroller.hasVideo()) {
			Bitmap *frame = _videocontroller.GetNextFrame();
			resultPtr = frame;
		}
	}
	else if (s.StartsWith("suggestFrame"))
	{
		int index = s.RemovePrefix("suggestFrame").ConvertToInteger();
		resultPtr = _videocontroller.GetSuggestionImage(index);
	}

	if(resultPtr == NULL) return NULL;

	resultPtr->FlipBlueAndRed();
	_queryBitmapInfo.width = resultPtr->Width();
	_queryBitmapInfo.height = resultPtr->Height();
	_queryBitmapInfo.colorData = (BYTE*)resultPtr->Pixels();
	return &_queryBitmapInfo;
}

BCBitmapInfo* App::SegmentImage(const BCBitmapInfo &bcbmp)
{
	Bitmap bmp;
	bmp.Allocate(bcbmp.width, bcbmp.height);
	for (UINT x=0; x<bcbmp.width; x++)
	{
		for (UINT y=0; y<bcbmp.height; y++)
		{
			int idx = 3*(y*bcbmp.width+x);
			bmp[y][x] = RGBColor(bcbmp.colorData[idx], bcbmp.colorData[idx+1], bcbmp.colorData[idx+2]);
		}
	}
	int numCCs; 
	Bitmap result = ImageToBitmap(segment_image(BitmapToImage(bmp), 0.5, 500, 20, &numCCs));

	BCBitmapInfo *info = new BCBitmapInfo();

	info->width = result.Width();
	info->height = result.Height();
	int width = info->width;
	int height = info->height;

	info->colorData = new byte[3*width*height];
	for (int x=0; x<width; x++)
	{
		for (int y=0; y<height; y++)
		{
			int idx = 3*(y*width+x);
			info->colorData[idx] = result[y][x].r; 
			info->colorData[idx+1] = result[y][x].g; 
			info->colorData[idx+2] = result[y][x].b; 
		}
	}


	return info;

}

int App::QueryIntegerByName(const String &s)
{
	if(s == "layerCount")
	{
		return 1;
	}
	else
	{
		SignalError("Unknown integer");
		return -1;
	}
}

const char* App::QueryStringByName(const String &s)
{
	if(s == "z")
	{
		_queryString = "a";
	}
	else
	{
		SignalError("Unknown string");
		return NULL;
	}
	return _queryString.CString();
}

void App::GetWords(const String& paletteFile)
{
	//file input is a list of image filename| palette rgb
	Vector<String> lines = Utility::GetFileLines(paletteFile);
	Vector<PixelLayerSet> images;

	int sample = Math::Min((int)lines.Length(), 500); //200 out of 1000

	//pick a random subset of the lines to get bag of words
	lines.Randomize();

	Console::WriteLine("Getting visual words...Extracting layers");
	for (int i=0; i<sample; i++)
	{
		const String& line = lines[i];
		Vector<String> fields = line.Partition("|");
		Vector<String> colors = fields[1].Partition(" ");
		String filename = fields[0];
		Console::WriteLine("File " + filename + " " + String(i)+"/"+String(sample));

		Bitmap bmp;
		bmp.LoadPNG(filename);

		Vector<Vec3f> palette;

		for (String& color:colors)
		{
			Vector<String> rgb = color.Partition(",");
			palette.PushEnd(Vec3f(rgb[0].ConvertToFloat()/255.f, rgb[1].ConvertToFloat()/255.f, rgb[2].ConvertToFloat()/255.f));
		}
		PixelLayerSet layers = ExtractLayers(bmp, palette, "", false);
		images.PushEnd(layers);
		bmp.FreeMemory();
	}
	Console::WriteLine("Extracting words");
	BagOfWords bow(_parameters, images, 100, true);
}

void App::OutputMesh(const BCBitmapInfo &bcbmp, const Vector<Vec3f> &palette, const String &filename)
{	
	Console::WriteLine("Extracting mesh to "+filename);
	/*ifstream File(filename.CString());
	if (File)
	{
	Console::WriteLine("File already exists");
	return;
	}*/

	Bitmap bmp;
	bmp.Allocate(bcbmp.width, bcbmp.height);
	for (UINT x=0; x<bcbmp.width; x++)
	{
		for (UINT y=0; y<bcbmp.height; y++)
		{
			int idx = 3*(y*bcbmp.width+x);
			bmp[y][x] = RGBColor(bcbmp.colorData[idx], bcbmp.colorData[idx+1], bcbmp.colorData[idx+2]);
		}
	}

	PixelLayerSet layers = ExtractLayers(bmp, palette, "", false);
	Console::WriteLine("Extracting mesh");
	LayerMesh mesh(layers);
	Console::WriteLine("Saving mesh");
	mesh.SaveToFile(filename);
	bmp.FreeMemory();
}

void App::LoadVideo(const String &filename, int paletteSize)
{
	_videocontroller.LoadVideo(filename, paletteSize);
}

byte App::GetVideoPalette(int paletteindex, int index)
{
	return _videocontroller.GetPalette(paletteindex, index);
}

void App::SetVideoPalette(int paletteindex, byte r, byte g, byte b)
{
	_videocontroller.SetPalette(paletteindex, r, g, b);
}

byte App::GetOriginalVideoPalette(int paletteindex, int index)
{
	return _videocontroller.GetOriginalPalette(paletteindex, index);
}

void App::SaveVideoFrames( void )
{
	_videocontroller.SaveFrames("../VideoResults/");
}

int App::GetVideoPaletteSize( void )
{
	return _videocontroller.getPaletteSize();
}

void App::SetVideoPreviewLayerIndex( int index )
{
	_videocontroller.SetPreviewLayerIndex(index);
}

void App::saveVideoPaletteImage( void )
{
	_videocontroller.SavePaletteImage("../VideoResults/");
}

int App::GetVideoHeight(void)
{
	return _videocontroller.Height();
}
int App::GetVideoWidth(void)
{
	return _videocontroller.Width();
}

int App::LoadSuggestions(void)
{
	return _videocontroller.LoadSuggestions();
}

void App::LoadSuggestion(int index)
{
	_videocontroller.LoadSuggestion(index);
}

byte App::GetSuggestPalette(int index, int paletteindex, int channel)
{
	return _videocontroller.GetSuggestPalette(index, paletteindex, channel);
}