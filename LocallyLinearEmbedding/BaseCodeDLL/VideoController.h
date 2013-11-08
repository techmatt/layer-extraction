
class VideoController
{
public:
	//VideoController(void);
	//~VideoController(void);

	void LoadVideo(String filename);

private:

	void ExtractVideoLayers(void);

	AppParameters _parameters;

	Video _video;
	UINT _frameCount;
	LayerSet _layers;
	LayerExtractorVideo _videoExtractor;
};

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
	}

	String videoFramesDirectory;
	String videoName;
	int zeropad; // video frame numbering
	int frameCount;
	float fps;
};