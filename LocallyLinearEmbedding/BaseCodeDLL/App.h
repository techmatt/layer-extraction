class App
{
public:
    void Init();

    UINT32 ProcessCommand(const String &command);
    BCBitmapInfo* QueryBitmapByName(const String &s);
    int QueryIntegerByName(const String &s);
    const char *QueryStringByName(const String &s);
	BCLayers* ExtractLayers(const BCBitmapInfo &bitmap, const Vector<Vec3f> &palette, const String &constraints, const bool autoCorrect);
	BCBitmapInfo* SegmentImage(const BCBitmapInfo &bitmap);
    BCLayers* SynthesizeLayers();

	void LoadVideo(const String &filename, int paletteSize);
	byte GetVideoPalette(int paletteindex, int index);
	void SetVideoPalette(int paletteindex, byte r, byte g, byte b);
	byte GetOriginalVideoPalette(int paletteindex, int index);

	void OutputMesh(const BCBitmapInfo &bcbmp, const Vector<Vec3f> &palette, const String &filename);

private:
    void Recolorize();
	//BCLayers* SynthesizeLayers();
	void SynthesizeTexture(const String &parameterFilename);
	void SynthesizeTextureByLayers(const String &parameterFilename);
	void DeleteLayer(const String &parameterFilename);
	void ExtractVideoLayers();
    
    AppParameters _parameters;

    Bitmap _image;
	LayerSet _layers;

    LayerExtractor _extractor;

	VideoController _videocontroller;

    String _queryString;

    BCBitmapInfo _queryBitmapInfo;
    Bitmap _queryBitmapResultA;
    Bitmap _queryBitmapResultB;
    Bitmap _queryBitmapResultC;

	BCLayers* PixelLayersToBCLayers(const PixelLayerSet &layers);

	PixelLayerSet ExtractLayers(const Bitmap &bitmap, const Vector<Vec3f> &palette, const String &constraints, const bool autoCorrect);
};
