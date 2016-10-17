
#include "main.h"

void App::go()
{
	explorer.init();
	explorer.layers.saveDAT(R"(C:\Code\layer-extraction\Images\les-miserables-layers\)");
	
	LightingConstraints constraints;

	const float startWeight = 0.01f;
	const float targetWeight = 1.0f;
	const Bitmap startImage = LodePNG::load(R"(C:\Code\layer-extraction\Images\les-miserables-start.png)");
	const Bitmap targetImage = LodePNG::load(R"(C:\Code\layer-extraction\Images\les-miserables-target.png)");

	constraints.init(startImage, startWeight, targetImage, targetWeight);
	constraints.saveDebug();

	explorer.populateCandidates(constraints);
}
