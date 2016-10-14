
#include "main.h"

Bitmap LightUtil::gridToBitmap(const Grid2<vec3f> &g)
{
	Bitmap result(g.getDimX(), g.getDimY());
	for (auto &v : result)
	{
		v.value = colorToVec4uc(g(v.x, v.y));
	}
	return result;
}