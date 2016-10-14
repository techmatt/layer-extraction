
#include "main.h"

void App::go()
{
	layers.load(R"(C:\Code\layer-extraction\Images\les-miserables-layers\)");

	vector<vec3f> colors;
	for (auto &l : layers.layers)
		colors.push_back(l.baseColor);

	Bitmap bmp = layers.compositeImage(colors);
	LodePNG::save(bmp, R"(C:\Code\layer - extraction\Images\test.png)");
}