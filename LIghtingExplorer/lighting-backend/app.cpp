
#include "main.h"

void App::go()
{
	explorer.init();
	explorer.fullLayers.saveDAT(R"(C:\Code\layer-extraction\Images\les-miserables-layers\)");
	explorer.smallLayers.saveDAT(R"(C:\Code\layer-extraction\Images\les-miserables-layers-small\)");
	
	LightingConstraints constraints;

	const float startWeight = 0.001f;
	const float targetWeight = 1.0f;
	Bitmap startImage = LodePNG::load(R"(C:\Code\layer-extraction\Images\les-miserables-start.png)");
	Bitmap targetImage = LodePNG::load(R"(C:\Code\layer-extraction\Images\les-miserables-target.png)");

	startImage = LightUtil::downsampleBitmap(startImage, Constants::smallLayersBlockSize);
	//LodePNG::save(startImage, params().debugDir + "start-downsampled.png");
	
	//targetImage = LightUtil::downsampleBitmap(targetImage, Constants::smallLayersBlockSize);

	constraints.init(startImage, startWeight, targetImage, targetWeight);
	constraints.saveDebug();

	explorer.populateCandidates(constraints);
	const Bitmap bmpFinal = explorer.fullLayers.compositeImage(explorer.candidateSamples[0].lightColors);
	LodePNG::save(bmpFinal, params().debugDir + "final.png");
}
