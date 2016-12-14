
#include "main.h"

void App::stallUntilFileExists(const string &filename)
{
	cout << "Waiting for update..." << endl;
	while (true)
	{
		if (util::fileExists(filename))
		{
			cout << "update found" << endl;
			return;
		}
		Sleep(100);
	}
}

void App::stagingMode()
{
	Recolorizer recolorizer;

	while (true)
	{
		stallUntilFileExists(appParams().stagingDir + "update.txt");

		ParameterFile newParams(appParams().stagingDir + "recoloringParamsFromUI.txt");
		newParams.readParameter("inputImage", appParamsMutable().inputImage);
		newParams.readParameter("editImage", appParamsMutable().editImage);

		const Bitmap imgInput = LodePNG::load(appParams().inputImage);
		const Bitmap imgEdits = LodePNG::load(appParams().editImage);

		const string cacheFilename = util::replace(appParams().inputImage, ".png", ".cache");
		recolorizer.init(imgInput, cacheFilename);

		const Bitmap result = recolorizer.recolor(imgEdits);
		LodePNG::save(result, util::replace(appParams().inputImage, ".png", "-result.png"));

		util::deleteFile(appParams().stagingDir + "update.txt");
	}
}

void App::directMode()
{
	const Bitmap imgInput = LodePNG::load(appParams().inputImage);
	const Bitmap imgEdits = LodePNG::load(appParams().editImage);

	Recolorizer recolorizer;
	const string cacheFilename = util::replace(appParams().inputImage, ".png", ".cache");
	recolorizer.init(imgInput, cacheFilename);

	const Bitmap result = recolorizer.recolor(imgEdits);
	LodePNG::save(result, util::replace(appParams().inputImage, ".png", "-result.png"));
}

void App::go()
{
	if (appParams().stagingMode)
	{
		stagingMode();
	}
	else
	{
		directMode();
	}
}
