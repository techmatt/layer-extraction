
struct Constants
{
	static const int a = 160;
};

struct AppParameters
{
	AppParameters()
	{
		debugDir = R"(TODO)";
		dataDir = R"(C:\Code\text2scene\data\SEL\)";
	}

	string debugDir;
	string dataDir;
};

extern AppParameters* g_appParams;
inline const AppParameters& appParams()
{
	return *g_appParams;
}

inline AppParameters& appParamsMutable()
{
	return *g_appParams;
}
