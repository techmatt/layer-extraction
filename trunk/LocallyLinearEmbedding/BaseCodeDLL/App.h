class App
{
public:
    void Init();

    UINT32 ProcessCommand(const String &command);
    BCBitmapInfo* QueryBitmapByName(const String &s);
    int QueryIntegerByName(const String &s);
    const char *QueryStringByName(const String &s);
	BCLayers* ExtractLayers(const BCBitmapInfo &bitmap, const Vector<Vec3f> &palette, const String &constraints, const bool autoCorrect, const String& imageFile);
	BCBitmapInfo* SegmentImage(const BCBitmapInfo &bitmap);
    BCLayers* SynthesizeLayers();

	void LoadVideo(const String &filename, int paletteSize);
	int GetVideoPaletteSize(void);
	byte GetVideoPalette(int paletteindex, int index);
	void SetVideoPalette(int paletteindex, byte r, byte g, byte b);
	byte GetOriginalVideoPalette(int paletteindex, int index);
	void SaveVideoFrames(void);
	void saveVideoPaletteImage(void);
	void SetVideoPreviewLayerIndex(int index);
	int GetVideoHeight(void);
	int GetVideoWidth(void);
	int LoadSuggestions(void);
	void LoadSuggestion(int index);
	byte GetSuggestPalette(int index, int paletteindex, int channel);

	void GetWords(const String &paletteFile);
	void OutputMesh(const BCBitmapInfo &bcbmp, const Vector<Vec3f> &palette, const String &filename);

private:
	void Recolorize();
	void SynthesizeTexture(const String &parameterFilename);
	void SynthesizeTextureByLayers(const String &parameterFilename);
	void DeleteLayer(const String &parameterFilename);
	void ExtractVideoLayers();
	void CacheLayers(const Vector<Vec3f> &palette, const Vector<PixelLayer> &pixellayers, const String& imageFile);

	void FilterLayers(const String &parameterFilename);
	bool ReadLayersFromCache(const String& cache, PixelLayerSet &layers, Vector<Vec3f>& palette, const Vector<PixelConstraint>& constraints);
	void ComputeLayers(const String& cache, const Bitmap& image, PixelLayerSet &layers, Vector<Vec3f>& palette, const Vector<PixelConstraint>& constraints);
    
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
