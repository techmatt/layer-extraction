#include "Main.h"

void App::Init()
{
    AllocConsole();
    _parameters.Init("../Parameters.txt");

    Bitmap bmp;
    //bmp.LoadPNG("../Data/bird.png");
    //bmp.LoadPNG("../Data/fish.png");
    //bmp.LoadPNG("../Data/bird.png");
	bmp.LoadPNG("../Data/2508514.png");
    //bmp.LoadPNG("../Data/princess.png");
    //bmp.LoadPNG("../Data/fish.png");
    //bmp.LoadPNG("../Data/throne.png");
    //bmp.LoadPNG("../Data/swordsman.png");
    //bmp.LoadPNG("../Data/faceCroppedSmall.png");

    Bitmap mask;
    //mask.LoadPNG("../Data/birdLayerA.png");
	mask.LoadPNG("../Data/2508514_Mask.png");

    _image = bmp;
    
    //_recolorizer.Init(_parameters, bmp);
    _extractor.Init(_parameters, bmp);

    Vector<PixelConstraint> pixelColors;
    Utility::AddMaskConstraints(bmp, mask, pixelColors);

    
    LayerSet layers;
    _extractor.InitLayers(_parameters, bmp, pixelColors, layers);
    _extractor.ExtractLayers(_parameters, bmp, layers);
    _extractor.AddNegativeConstraints(_parameters, bmp, layers);
    _extractor.ExtractLayers(_parameters, bmp, layers);
    
    //Bitmap result = _recolorizer.Recolor(_parameters, bmp, pixelColors, 0.001, 0.6);
    //result.SavePNG("../Results/result.png");

    /*int cutoffIndex = 0;
    for(double cutoff = 0.1; cutoff <= 0.85; cutoff += 0.05, cutoffIndex++)
    //for(double cutoff = 0.0; cutoff <= 0.1; cutoff += 0.01, cutoffIndex++)
    {
        Bitmap result = _recolorizer.Recolor(_parameters, bmp, pixelColors, 0.001, cutoff);
        result.SavePNG("../Results/result" + String(cutoffIndex) + "_" + String(cutoff) + ".png");
    }*/

    /*for(const PixelConstraint &c : pixelColors)
    {
        if(bmp[c.coord.y][c.coord.x] == RGBColor(c.targetColor)) result[c.coord.y][c.coord.x] = RGBColor::Black;
        else result[c.coord.y][c.coord.x] = RGBColor(c.targetColor);
    }

    result.SavePNG("../Results/resultAnnotated.png");*/
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
