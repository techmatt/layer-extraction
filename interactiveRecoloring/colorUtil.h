
class colorUtil
{
public:
	static vec3f toVec3f(const vec4uc &c)
	{
		return vec3f(c.x / 255.0f, c.y / 255.0f, c.z / 255.0f);
	}

	static vec4uc toVec4uc(const vec3f &c)
	{
		return vec4uc(util::boundToByte(c.x * 255.0f),
					  util::boundToByte(c.y * 255.0f),
					  util::boundToByte(c.z * 255.0f),
					  255);
	}

	static vec3f randomColorVec3f()
	{
	
		//vec3f::dist
		//vec3f r(math::random_uniform(), math::random_uniform(), math::random_uniform());
		vec3f r;
		do {
			r = vec3f::randomUniform(0.0f, 1.0f);
		} while (r.lengthSq() < 0.7f * 0.7f);
		return r;
	}

	static vec4uc randomColorVec4uc()
	{
		return toVec4uc(randomColorVec3f());
	}

};