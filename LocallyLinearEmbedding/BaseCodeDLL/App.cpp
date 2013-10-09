#include "Main.h"
#include "segment-image.h"

const bool usingRecolorizer = false;

void App::Init()
{
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
	_layers = layers;
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

BCLayers* App::ExtractLayers(const BCBitmapInfo &bcbmp, const Vector<Vec3f> &palette, const String &constraints)
{
	 AllocConsole();

     Vector<int> v(5);
    for(int i = 0; i < 5; i++) v[i] = i;
    auto mappedVector = v.Map(function<int(int)>([](int a) { return a * a; }));

#ifdef DEBUG
     Console::WriteLine("DLL compiled in release mode");
#else
     Console::WriteLine("DLL compiled in debug mode");
#endif

    _parameters.Init("../Parameters.txt");
	
    if(usingRecolorizer)
    {
        Recolorize();
    }

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

	_layers = layers;

	Vector<PixelLayer> pixellayers = _extractor.GetPixelLayers(_image, _layers);
	BCLayers *result = PixelLayersToBCLayers(pixellayers);

	return result;
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




UINT32 App::ProcessCommand(const String &command)
{
	if (command == "SynthesizeTexture") {
		// params
		_parameters.Init("../Parameters.txt");
		int reducedDimension = _parameters.texsyn_pcadim;
		int neighborhoodSize = _parameters.texsyn_neighbourhoodsize; // radius
		int outputwidth = _parameters.texsyn_outputwidth;
		int outputheight = _parameters.texsyn_outputheight;
		int nlevels = _parameters.texsyn_nlevels;
		int kcoh = _parameters.texsyn_coherence;
		int ncoh = _parameters.texsyn_coherenceNeighbourhoodSize;
		double kappa = _parameters.texsyn_kappa;
		
		String layerDir = "../Layers/";
		String exemplar_name = "../TextureSynthesisExemplars/" + _parameters.texsyn_exemplar;
		// read in layers
		PixelLayerSet input;
		/*for (UINT i=0; i<_parameters.targetLayers.Length(); i++)
		    input.PushEnd(PixelLayer(layerDir+_parameters.targetLayers[i]));*/
		
		Console::WriteLine("Texture Synthesis: " + exemplar_name);

		Bitmap rgbimg;
		rgbimg.LoadPNG(exemplar_name);
		//debugging
		rgbimg.SavePNG("texsyn-out/input.png");
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
		input.PushEnd(red);
		input.PushEnd(green);
		input.PushEnd(blue);
		
		NeighborhoodGenerator generator(neighborhoodSize, input.Length(), 1);
		GaussianPyramid pyramid(input, nlevels);
		//pyramid.Write(String("texsyn-out/pyr"));
		TextureSynthesis synthesizer;
		synthesizer.Init(_parameters.texsyn_exemplar, pyramid, generator, nlevels, reducedDimension, kcoh, ncoh);
		synthesizer.Synthesize(pyramid, outputwidth, outputheight, generator, kappa);
	}
    return 0;
}

BCBitmapInfo* App::QueryBitmapByName(const String &s)
{
    const UINT width = _image.Width();
    const UINT height = _image.Height();
    
    Bitmap *resultPtr = &_image;

    resultPtr->FlipVertical();
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
