#include "Main.h"
#include "segment-image.h"

void App::Init()
{
    AllocConsole();
    _parameters.Init("../Parameters.txt");

    Bitmap bmp;
    bmp.LoadPNG("../Data/" + _parameters.imageFile);
    
    _image = bmp;
    
    //_recolorizer.Init(_parameters, bmp);
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
    
    //Bitmap result = _recolorizer.Recolor(_parameters, bmp, pixelColors, 0.001, 0.6);
    //result.SavePNG("../Results/result.png");
}

BCLayers* App::ExtractLayers(BCBitmapInfo bcbmp, Vector<Vec3f> palette)
{
	 AllocConsole();
    _parameters.Init("../Parameters.txt");
	

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

	_extractor.InitLayersFromPalette(_parameters, bmp, palette, layers);

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


	BCLayers *result = new BCLayers();

	Vector<PixelLayer> pixellayers = _extractor.GetPixelLayers(_image, _layers);
	result->layers = new BCLayerInfo[pixellayers.Length()];
	result->numLayers = pixellayers.Length();

	UINT width = pixellayers.First().width;
	UINT height = pixellayers.First().height;
	
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



UINT32 App::ProcessCommand(const String &command)
{
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

BCBitmapInfo* App::SegmentImage(BCBitmapInfo bcbmp)
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
