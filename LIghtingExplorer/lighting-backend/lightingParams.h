
struct Constants
{
	static constexpr double signatureDistScale = 10000.0;

	static constexpr int smallLayersBlockSize = 4;
	static constexpr int signatureBlockSize = 2;
	static constexpr float finalExclusionRadius = 0.75f;
	static constexpr float acceptanceExclusionRadius = 1.0f;
	static constexpr float fitnessExclusionRadius = 2.0f;
	static constexpr double exclusionStrength = 10.0;
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
