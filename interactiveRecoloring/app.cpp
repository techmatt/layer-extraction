
#include "main.h"

void App::go()
{
	const Bitmap imgInput = LodePNG::load(appParams().inputImage);
	const Bitmap imgEdits = LodePNG::load(appParams().editImage);

	Recolorizer recolorizer;
	recolorizer.init(imgInput);

	const Bitmap result = recolorizer.recolor(imgEdits);
	LodePNG::save(result, util::replace(appParams().inputImage, ".png", "-result.png"));
}
