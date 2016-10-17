
struct LightUtil
{
	static vec4uc colorToVec4uc(const vec3f &v)
	{
		return vec4uc(util::boundToByte(v.x * 255.0f),
					  util::boundToByte(v.y * 255.0f),
					  util::boundToByte(v.z * 255.0f), 255);
	}
	static vec3f vec4ucToColorVec3(const vec4uc &v)
	{
		return vec3f(v.getVec3()) / 255.0f;
	}
	static Bitmap gridToBitmap(const Grid2<vec3f> &g);

	static vector<float> lightsToRaw(const vector<vec3f> &lights);
	static vector<vec3f> rawToLights(const vector<float> &raw);
};
