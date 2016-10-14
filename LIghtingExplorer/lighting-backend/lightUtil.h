
struct LightUtil
{
	static vec4uc colorToVec4uc(const vec3f &v)
	{
		return vec4uc(util::boundToByte(v.x * 255.0f),
					  util::boundToByte(v.y * 255.0f),
					  util::boundToByte(v.z * 255.0f), 255);
	}
	static Bitmap gridToBitmap(const Grid2<vec3f> &g);
};