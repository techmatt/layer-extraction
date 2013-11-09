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

		videoPaletteFile = file.GetString("videoPaletteFile", "");

		videoSaveName = file.GetString("videoSaveName", videoName);
	}

	String videoFramesDirectory;
	String videoName;
	int zeropad; // video frame numbering
	int frameCount;
	float fps;

	String videoPaletteFile;

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

	//Video _video;
	UINT _videoWidth, _videoHeight, _frameCount;
	Vector<Vec3f> _palette;
	//LayerSet _layers;
	Vector<PixelLayerSet> _players;
	LayerExtractorVideo _videoExtractor;

	int _currFrameIndex;
	Bitmap *_frameA, *_frameB;
	bool _usingframeA;

	bool _hasVideo;
};

