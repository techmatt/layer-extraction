
#include "main.h"

Bitmap LightUtil::gridToBitmap(const Grid2<vec3f> &g)
{
	Bitmap result(g.getDimX(), g.getDimY());
	for (auto &v : result)
	{
		v.value = toColorVec4uc(g(v.x, v.y));
	}
	return result;
}

vector<float> LightUtil::lightsToRaw(const vector<vec3f>& lights)
{
	vector<float> result;
	for (auto &v : lights)
	{
		result.push_back(v.x);
		result.push_back(v.y);
		result.push_back(v.z);
	}
	return result;
}

vector<vec3f> LightUtil::rawToLights(const vector<float>& raw)
{
	vector<vec3f> lights;
	for (size_t i = 0; i < raw.size(); i += 3)
	{
		const vec3f v(raw[i + 0], raw[i + 1], raw[i + 2]);
		lights.push_back(v);
	}
	return lights;
}

Bitmap LightUtil::downsampleBitmap(const Bitmap &bmp, int blockSize)
{
	const double scale = 1.0 / (blockSize * blockSize);

	int newDimX = bmp.getDimX() / blockSize;
	int newDimY = bmp.getDimY() / blockSize;

	Grid2<vec3f> result(newDimX, newDimY);

	for (auto &p : result)
	{
		vec3f sum = vec3f::origin;
		for (int yOffset = 0; yOffset < blockSize; yOffset++)
			for (int xOffset = 0; xOffset < blockSize; xOffset++)
				sum += LightUtil::toColorVec3(bmp(p.x * blockSize + xOffset, p.y * blockSize + yOffset));
		p.value = sum * scale;
	}

	return LightUtil::gridToBitmap(result);
}
