class App
{
public:
    void Init();

    UINT32 ProcessCommand(const String &command);
    BCBitmapInfo* QueryBitmapByName(const String &s);
    int QueryIntegerByName(const String &s);
    const char *QueryStringByName(const String &s);
	BCLayers* GetLayers();
    
private:
    AppParameters _parameters;

    Bitmap _image;
	LayerSet _layers;

    LayerExtractor _extractor;
    //Recolorizer _recolorizer;

    String _queryString;

    BCBitmapInfo _queryBitmapInfo;
    Bitmap _queryBitmapResultA;
    Bitmap _queryBitmapResultB;
    Bitmap _queryBitmapResultC;
};
