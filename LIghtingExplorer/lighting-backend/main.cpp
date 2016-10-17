
#include "main.h"

LightingParameters *g_lightingParams;

void main()
{
	g_lightingParams = new LightingParameters();

	App app;
	app.go();
	
	cout << "Done!" << endl;
	cin.get();
}
