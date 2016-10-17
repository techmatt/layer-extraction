
struct LightingParameters
{
	LightingParameters()
	{
		debugDir = R"(C:\Code\layer-extraction\Images\debug\)";

	}
	string debugDir;
};

extern LightingParameters* g_lightingParams;
inline const LightingParameters& params()
{
	return *g_lightingParams;
}

inline LightingParameters& paramsMutable()
{
	return *g_lightingParams;
}
