class App
{
public:
    void Init();

    UINT32 ProcessCommand(const String &command);
    BCBitmapInfo* QueryBitmapByName(const String &s);
    int QueryIntegerByName(const String &s);
    const char *QueryStringByName(const String &s);
	BCLayers* ExtractLayers(BCBitmapInfo bitmap, Vector<Vec3f> palette);
	BCBitmapInfo* SegmentImage(BCBitmapInfo bitmap);
    void Recolorize();
    
private:
    AppParameters _parameters;

    Bitmap _image;
	LayerSet _layers;

    LayerExtractor _extractor;

    String _queryString;

    BCBitmapInfo _queryBitmapInfo;
    Bitmap _queryBitmapResultA;
    Bitmap _queryBitmapResultB;
    Bitmap _queryBitmapResultC;
};
