#include "Main.h"

BASECODEDLL_API void* __stdcall BCInit()
{
    App *app = new App;
    //app->Init();
	AllocConsole();
    return app;
}

BASECODEDLL_API UINT32 __stdcall BCProcessCommand(void *context, const char *s)
{
    if(context == NULL) return 1;
    App &app = *(App*)context;
    UINT32 result = app.ProcessCommand(String(s));
    return result;
}

BASECODEDLL_API const char* __stdcall BCQueryStringByName(void *context, const char *s)
{
    if(context == NULL) return NULL;
    App &app = *(App*)context;
    return app.QueryStringByName(s);
}

BASECODEDLL_API int __stdcall BCQueryIntegerByName(void *context, const char *s)
{
    if(context == NULL) return 0;
    App &app = *(App*)context;
    return app.QueryIntegerByName(s);
}

BASECODEDLL_API BCBitmapInfo* __stdcall BCQueryBitmapByName(void *context, const char *s)
{
    if(context == NULL) return 0;
    App &app = *(App*)context;
    return app.QueryBitmapByName(s);
}


BASECODEDLL_API BCLayers* __stdcall BCExtractLayers(void *context, BCBitmapInfo bitmap, const double* palette, int paletteSize, const char *constraints)
{
	if (context == NULL) return 0;
	App *app = (App*) context;

	Vector<Vec3f> p;
	int numColors = paletteSize;
	double* data = (double*)palette;

	for (int i=0; i<numColors; i++)
		p.PushEnd(Vec3f((float)data[3*i], (float)data[3*i+1], (float)data[3*i+2]));
	
	return app->ExtractLayers(bitmap, p, constraints);
}

BASECODEDLL_API BCBitmapInfo* __stdcall BCSegmentImage(void* context, BCBitmapInfo bitmap)
{
	if (context == NULL) return 0;
	App *app = (App*) context;
	

	return app->SegmentImage(bitmap);

}