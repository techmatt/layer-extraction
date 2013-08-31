#include "Main.h"

void App::Init()
{
    AllocConsole();
    _parameters.Init("../Parameters.txt");

    Bitmap bmp;
    bmp.LoadPNG("../Data/" + _parameters.imageFile);
    
    Bitmap mask;
    mask.LoadPNG("../Data/" + _parameters.maskFile);
    
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
    _extractor.AddNegativeConstraints(_parameters, bmp, layers);
    _extractor.ExtractLayers(_parameters, bmp, layers);
    _extractor.AddNegativeConstraints(_parameters, bmp, layers);
    _extractor.ExtractLayers(_parameters, bmp, layers);
    _extractor.AddNegativeConstraints(_parameters, bmp, layers);
    _extractor.ExtractLayers(_parameters, bmp, layers);

    layers.Dump("../Results/Layers.txt", _extractor.SuperpixelColors());

    _extractor.TestLayerRecoloring(bmp, layers);
    
    //Bitmap result = _recolorizer.Recolor(_parameters, bmp, pixelColors, 0.001, 0.6);
    //result.SavePNG("../Results/result.png");
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
