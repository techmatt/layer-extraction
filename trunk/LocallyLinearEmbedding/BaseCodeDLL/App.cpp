#include "Main.h"
#include "segment-image.h"

const bool usingRecolorizer = false;

void App::Init()
{
	/*AllocConsole();
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

BCLayers* App::ExtractLayers(const BCBitmapInfo &bcbmp, const Vector<Vec3f> &palette, const String &constraints, const bool autoCorrect)
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

	BCLayers *result = PixelLayersToBCLayers(pixellayers);

	return result;
}

PixelLayerSet App::ExtractLayers(const Bitmap &bmp, const Vector<Vec3f> &palette, const String &constraints, const bool autoCorrect)
{
#ifdef _DEBUG
	Console::WriteLine("DLL compiled in release mode");
#else
	Console::WriteLine("DLL compiled in debug mode");
#endif

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

	for(UINT negativeSuppressionIndex = 0; negativeSuppressionIndex < 4; negativeSuppressionIndex++)
	{
		_extractor.ExtractLayers(_parameters, bmp, layers);
		_extractor.AddNegativeConstraints(_parameters, bmp, layers);
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

	//videoExtractor.InitLayersFromPalette(_parameters, video, video.ComputePaletteKMeans(6), layers);
	videoExtractor.InitLayersFromPalette(_parameters, video, video.ComputeFrame0Palette("../Data/sintel-5/sintel-5_00_palette.png"), layers);
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

void App::DeleteLayer(const String &parameterFilename)
{
	_parameters.Init("../Parameters.txt");

	const int klayers = 6;
	const int idx = 1; // layer to delete
	bool del = false;
	String imagename = "reflection.png";
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
	for (int y = 0; y < image.Height(); y++) {
		for (int x = 0; x < image.Width(); x++) {
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
		for (int y = 0; y < image.Height(); y++) {
			for (int x = 0; x < image.Width(); x++) {
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
		for (int y = 0; y < image.Height(); y++) {
			for (int x = 0; x < image.Width(); x++) {
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
		if (!cached) {
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
		Vector<float> scaled(klayers-1);
		int i;
		for (int y = 0; y < image.Height(); y++) {
			for (int x = 0; x < image.Width(); x++) {
				Vec3f v(0,0,0);
				i = 0;
				scale = 0;
				for (int k = 0; k < klayers; k++) {
					if (k != idx) {
						scaled[i] = (float) layers[k].pixelWeights(y,x) * nlayers[i].pixelWeights(y,x);
						scale += scaled[i];
						i++;
					}
				}
				if (scale < FLT_EPSILON) scale = 1;
				for (int k = 0; k < klayers-1; k++) 
					v += nlayers[k].color * scaled[k] / scale;
				output[y][x] = RGBColor(v);
			}
		}
		output.SavePNG(outdir + "output-scaled-2.png");
	}

	Console::WriteLine("Done!");
}

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
		for (int i = 0; i < rgblayers.Length(); i++) {
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
		DeleteLayer(words[1]);
	}
	else if (words[0] == "ExtractVideoLayers") {
		ExtractVideoLayers();
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
}

void App::LoadVideo(const String &filename, int paletteSize)
{
	_videocontroller.LoadVideo(filename, paletteSize);
}
