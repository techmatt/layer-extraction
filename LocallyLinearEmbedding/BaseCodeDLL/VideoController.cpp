#include "Main.h"


VideoController::VideoController(void)
{
	_frameA = NULL;
	_frameB = NULL;
	_frameCount = 0;
	_videoWidth = 0;
	_videoHeight = 0;
	_previewLayerIndex = -1;
	_currFrameIndex = -1;
	_hasVideo = false;
}

VideoController::~VideoController(void)
{
	if (_frameA) delete _frameA;
	if (_frameB) delete _frameB;
}

void VideoController::LoadVideo(const String& filename, int paletteSize)
{
	_parameters.Init("../Parameters.txt"); 
	_vidparams.Init(filename);

	// read in video
	Video video;
	LoadVideoFromParams(video);

	_currFrameIndex = -1;

	// extract video layers
	ExtractVideoLayers(video, paletteSize);

	_hasVideo = true;
}

void VideoController::SetPalette(int paletteindex, byte r, byte g, byte b)
{
	Assert(paletteindex >= 0 && paletteindex < _palette.Length(), "trying to set palette index out of bounds");
	_palette[paletteindex] = Vec3f(RGBColor(r, g, b));
}

Bitmap* VideoController::GetNextFrame(void)
{
	if (!_hasVideo) return NULL;

	if (!_frameA || !_frameB) {
		if (_frameA) delete _frameA;
		if (_frameB) delete _frameB;
		_frameA = new Bitmap(_videoWidth, _videoHeight);
		_frameB = new Bitmap(_videoWidth, _videoHeight);
		_usingframeA = false;
	}
	Bitmap *currframe = _frameA;
	if (_usingframeA) currframe = _frameB;

	_currFrameIndex = (_currFrameIndex + 1) % _frameCount;
	Assert(_currFrameIndex >= 0 && _currFrameIndex < _frameCount, "frame index out of bounds!");

	if (_previewLayerIndex < 0) { // full colour
		for (int y = 0; y < _videoHeight; y++)
			for (int x = 0; x < _videoWidth; x++)
				(*currframe)[y][x] = GetColor(_currFrameIndex, x, y);
	}
	else {
		(*currframe).LoadGrid(_players[_currFrameIndex][_previewLayerIndex].pixelWeights);
	}

	_usingframeA = !_usingframeA;

	return currframe;
}

Bitmap* VideoController::GetCurrentFrame(void)
{
	if (!_hasVideo) return NULL;
	if (_currFrameIndex < 0) _currFrameIndex = 0;

	if (!_frameA || !_frameB) {
		if (_frameA) delete _frameA;
		if (_frameB) delete _frameB;
		_frameA = new Bitmap(_videoWidth, _videoHeight);
		_frameB = new Bitmap(_videoWidth, _videoHeight);
		_usingframeA = true;
	}
	Bitmap *currframe = _frameA;
	if (_usingframeA) currframe = _frameB;

	if (_previewLayerIndex < 0) { // full colour
		for (int y = 0; y < _videoHeight; y++)
			for (int x = 0; x < _videoWidth; x++)
				(*currframe)[y][x] = GetColor(_currFrameIndex, x, y);
	}
	else {
		(*currframe).LoadGrid(_players[_currFrameIndex][_previewLayerIndex].pixelWeights);
	}

	return currframe;
}

RGBColor VideoController::GetColor(UINT frameIndex, int x, int y)
{
	if (_players.Length() > 0) {
		Vec3f v(0,0,0);
		for (UINT layerIndex = 0; layerIndex < _palette.Length(); layerIndex++) 
			v += _palette[layerIndex] * _players[frameIndex][layerIndex].pixelWeights(y,x);
		return RGBColor(v);
	}
	return RGBColor(0,0,0); // todo!!!!
}


void VideoController::LoadVideoFromParams(Video &video)
{
	_frameCount = _vidparams.frameCount;

	for (UINT frameIndex = 0; frameIndex < _frameCount; frameIndex++)
	{
		Bitmap bmp;
		bmp.LoadPNG(_vidparams.videoFramesDirectory + _vidparams.videoName + String::ZeroPad(frameIndex, _vidparams.zeropad) + ".png");
		video.frames.PushEnd(bmp);
	}
	_videoWidth = video.Width();
	_videoHeight = video.Height();
	Console::WriteLine("loaded video frames");
}

void VideoController::ExtractVideoLayers(Video &video, UINT paletteSize)
{
	// get palette
	if (_vidparams.videoPaletteFile != "")
		_palette = video.ComputeFrame0Palette(_vidparams.videoPaletteFile);
	else {
		// todo: check if palette is cached !!!!!!
		_palette = video.ComputePaletteKMeans(paletteSize);
	}
	_players.Allocate(_frameCount);

	if (ReadLayersFromCache()) return;

	// compute layers
	LayerSet layers;
	_videoExtractor.Init(_parameters, video);
	_videoExtractor.InitLayersFromPalette(_parameters, video, _palette, layers);

	for(UINT negativeSuppressionIndex = 0; negativeSuppressionIndex < 4; negativeSuppressionIndex++)
	{
		_videoExtractor.ExtractLayers(_parameters, video, layers);
		_videoExtractor.AddNegativeConstraints(_parameters, video, layers);
	}

	Console::WriteLine("computing pixel layers...");
	for (UINT frameIndex = 0; frameIndex < _frameCount; frameIndex++)
		_players[frameIndex] = _videoExtractor.GetPixelLayers(video, frameIndex, layers);
	Console::WriteLine("caching layers...");
	CacheLayers();

	//WriteLayerFrames("../Results/VideoLayers/", video, layers);

	Console::WriteLine("Done extracting video layers");
}

void VideoController::CacheLayers(void)
{
	String cache = "../VideoCache/" + _vidparams.videoSaveName + ".dat";

	OutputDataStream fileBinary;

	fileBinary << _palette.Length() << _frameCount;

	for(UINT layerIndex = 0; layerIndex < _palette.Length(); layerIndex++)
	{
		//String layerDirectory = cache + "Layer" + String(layerIndex) + "/";
		//Utility::MakeDirectory(layerDirectory);

		for(UINT frameIndex = 0; frameIndex < _frameCount; frameIndex++) {
			const auto &curLayer = _players[frameIndex][layerIndex];
			//curLayer.WriteToFile(layerDirectory + "f" + String::ZeroPad(frameIndex, 4) + ".txt");
			fileBinary.WriteData(curLayer.color);
			fileBinary << curLayer.pixelWeights;
		}
	}

	fileBinary.SaveToCompressedFile(cache);
}

bool VideoController::ReadLayersFromCache(void)
{
	String cache = "../VideoCache/" + _vidparams.videoSaveName + ".dat";
	if (!Utility::FileExists(cache)) return false;

	InputDataStream fileBinary;
	fileBinary.LoadFromCompressed(cache);

	UINT paletteSize;
	fileBinary >> paletteSize;
	fileBinary >> _frameCount;

	for(UINT frameIndex = 0; frameIndex < _frameCount; frameIndex++)
		_players[frameIndex].Allocate(paletteSize);

	for(UINT layerIndex = 0; layerIndex < paletteSize; layerIndex++) {

		for(UINT frameIndex = 0; frameIndex < _frameCount; frameIndex++) {

			fileBinary.ReadData(_players[frameIndex][layerIndex].color);
			fileBinary >> _players[frameIndex][layerIndex].pixelWeights;
		}
	}
	Console::WriteLine("read layers from cache");
	return true;
}

void VideoController::WriteLayerFrames( const String& location, const Video &video, const LayerSet &layers )
{
	String resultdir = location + _vidparams.videoSaveName + "/";
	Utility::MakeDirectory(resultdir);

	for(UINT layerIndex = 0; layerIndex < _palette.Length(); layerIndex++)
	{
		String layerDirectory = resultdir + "Layer" + String(layerIndex) + "/";
		Utility::MakeDirectory(layerDirectory);

		for(UINT frameIndex = 0; frameIndex < _frameCount; frameIndex++)
		{
			Bitmap bmp = _videoExtractor.VisualizeLayer(_parameters, video, frameIndex, layerIndex, layers);
			bmp.SavePNG(layerDirectory + "/f" + String::ZeroPad(frameIndex, 4) + ".png");
		}
	}
}

byte VideoController::GetPalette( int paletteindex, int index )
{
	Assert(paletteindex >= 0 && paletteindex < _palette.Length() && index >= 0 && index < 3, "palette query out of bounds");
	return Utility::BoundToByte(_palette[paletteindex][index] * 255.0f);
}

byte VideoController::GetOriginalPalette( int paletteindex, int index )
{
	Assert(paletteindex >= 0 && paletteindex < _palette.Length() && index >= 0 && index < 3, "palette query out of bounds");
	return Utility::BoundToByte(_players[0][paletteindex].color[index] * 255.0f);
}

void VideoController::SaveFrames( const String& resultdirectory )
{
	Utility::MakeDirectory(resultdirectory);
	String saveDirectory = resultdirectory + _vidparams.videoSaveName + "/";
	Utility::MakeDirectory(saveDirectory);
	String saveLocation = saveDirectory + _vidparams.videoSaveName;

	Bitmap currFrame(_videoWidth, _videoHeight);
	for (int f = 0; f < _frameCount; f++) {
		for (int y = 0; y < _videoHeight; y++)
			for (int x = 0; x < _videoWidth; x++)
				currFrame[y][x] = GetColor(f, x, y);

		currFrame.SavePNG(saveLocation + "_" + String::ZeroPad(f, _vidparams.zeropad) + ".png");
	}
	Console::WriteLine("saved video to " + saveDirectory);
}

void VideoController::SetPreviewLayerIndex( int index )
{
	if (index >= -1 && index < (int)_palette.Length())
		_previewLayerIndex = index;
}

