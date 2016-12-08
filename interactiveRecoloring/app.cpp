
#include "main.h"

void App::go()
{
	const Bitmap imgEdits = LodePNG::load(appParams().editImage);
	
	ImageSuperpixels superpixels;

	superpixels.imgInput = LodePNG::load(appParams().inputImage);

	SuperpixelGeneratorSuperpixel generator;
	const vector<SuperpixelCoord> superpixelCoords = generator.extract(superpixels.imgInput, superpixels.assignments);

	superpixels.loadCoords(superpixelCoords);
	superpixels.computeNeighborhoods();
	superpixels.computeNeighborhoodWeights();

}
