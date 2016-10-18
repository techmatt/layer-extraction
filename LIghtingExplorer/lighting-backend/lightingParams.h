
struct Constants
{
	static constexpr int smallLayersBlockSize = 4;
	static constexpr int signatureBlockSize = 2;
	static constexpr float exclusionRadius = 0.01f;
	static constexpr double exclusionStrength = 1.0;
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
