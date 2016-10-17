
#include "main.h"

void ImageLayers::loadCSV(const string &baseDir)
{
	const int layerCount = Directory::enumerateFiles(baseDir, ".csv").size();
	layers.resize(layerCount);
	for (int i = 0; i < layerCount; i++)
	{
		auto &l = layers[i];

		auto lines = util::getFileLines(baseDir + "layer" + to_string(i) + ".csv", 3);
		auto colorParts = util::split(lines[0], ',');
		l.baseColor.x = convert::toFloat(colorParts[0]);
		l.baseColor.y = convert::toFloat(colorParts[1]);
		l.baseColor.z = convert::toFloat(colorParts[2]);

		if (i == 0)
		{
			dimX = util::split(lines[1], ",").size();
			dimY = lines.size() - 1;
		}

		l.g.allocate(dimX, dimY);
		for (int y = 0; y < dimY; y++)
		{
			auto parts = util::split(lines[y + 1], ',');
			for (int x = 0; x < dimX; x++)
			{
				l.g(x, y) = convert::toFloat(parts[x]);
			}
		}
	}
}

void ImageLayers::saveDAT(const string &baseDir) const
{
	for (size_t i = 0; i < layers.size(); i++)
	{
		auto &l = layers[i];
		
		Bitmap bmp(l.g.getDimensions());
		for (auto &p : bmp)
		{
			BYTE b = util::boundToByte(l.g(p.x, p.y) * 255.0f);
			p.value = vec4uc(b, b, b, 255);
		}
		LodePNG::save(bmp, baseDir + "layer" + to_string(i) + ".png");
		util::serializeToFile(baseDir + "layer" + to_string(i) + ".dat", l);
	}
}

Bitmap ImageLayers::compositeImage(const vector<vec3f>& layerColors) const
{
	Grid2<vec3f> result(dimX, dimY, vec3f::origin);
	for (auto &v : result)
	{
		vec3f c = vec3f::origin;
		for (int layerIndex = 0; layerIndex < layers.size(); layerIndex++)
		{
			c += layerColors[layerIndex] * layers[layerIndex].g(v.x, v.y);
		}
		v.value = c;
	}
	return LightUtil::gridToBitmap(result);
}
