struct VideoParameters
{
	void Init(const String &parameterFilename)
	{
		ParameterFile file(parameterFilename);

		videoFramesDirectory = file.GetRequiredString("videoFramesDirectory");
		videoName = file.GetRequiredString("videoName");
		zeropad = file.GetInteger("zeropad");
		frameCount = file.GetInteger("frameCount");
		fps = file.GetFloat("fps");

		videoPaletteFile.FreeMemory();
		Vector<String> videoPaletteIndices = file.GetString("videoPaletteFile", "").Partition(",");
		for (UINT i = 0; i < (UINT) videoPaletteIndices.Length(); i++)
			videoPaletteFile.PushEnd(videoPaletteIndices[i].ConvertToUnsignedInteger());

		videoSaveName = file.GetString("videoSaveName", videoName);
	}

	String videoFramesDirectory;
	String videoName;
	int zeropad; // video frame numbering
	int frameCount;
	float fps;

	Vector<UINT> videoPaletteFile;

	String videoSaveName;
};


class VideoController
{
public:
	VideoController(void);
	~VideoController(void);

	void LoadVideo(const String& filename, int paletteSize);
	void WriteLayerFrames( const String& location, const Video &video, const LayerSet &layers );
	Bitmap* GetNextFrame(void);
	Bitmap* GetCurrentFrame(void);
	int getPaletteSize(void) { return _palette.Length(); }
	byte GetOriginalPalette(int paletteindex, int index);
	byte GetPalette(int paletteindex, int index);
	void SetPalette(int paletteindex, byte r, byte g, byte b);
	void SaveFrames(const String& resultdirectory);
	void SavePaletteImage(const String& resultdirectory);
	void SetPreviewLayerIndex(int index);
	int Height(void) const { return (int)_videoHeight; }
	int Width(void) const { return (int)_videoWidth; }

	// suggestions
	int LoadSuggestions(void);
	Bitmap* GetSuggestionImage(int index);
	void LoadSuggestion(int index);
	byte GetSuggestPalette(int index, int paletteindex, int channel);

	//const Bitmap* GetNextFrameOriginal(void);
	//const Bitmap* GetCurrentFrameOriginal(void);

	bool hasVideo(void) { return _hasVideo; }

private:
	void LoadVideoFromParams(Video &video);
	void ExtractVideoLayers(Video &video, UINT paletteSize);
	RGBColor GetColor(UINT frameIndex, int x, int y);

	void CacheLayers(void);
	bool ReadLayersFromCache(void);

	AppParameters _parameters;
	VideoParameters _vidparams;

	// original
	UINT _videoWidth, _videoHeight, _frameCount;
	Vector<Vec3f> _palette;
	Vector<PixelLayerSet> _players;
	LayerExtractorVideo _videoExtractor;

	// suggestions
	Vector< Vector<Vec3f> > _suggestPalettes;
	Vector<Bitmap*> _suggestImages;

	int _currFrameIndex, _previewLayerIndex;
	Bitmap *_frameA, *_frameB;
	bool _usingframeA;

	bool _hasVideo;
};

