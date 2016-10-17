
struct Constants
{
	static constexpr int smallLayersBlockSize = 4;
	static constexpr int signatureBlockSize = 4;
};

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
