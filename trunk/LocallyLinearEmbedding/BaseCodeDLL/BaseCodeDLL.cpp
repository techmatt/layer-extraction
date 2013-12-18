#include "Main.h"

BASECODEDLL_API void* __stdcall BCInit()
{
    App *app = new App;
    AllocConsole();

#ifdef _DEBUG
    Console::WriteLine("DLL compiled in debug mode");
#else
    Console::WriteLine("DLL compiled in release mode");
#endif

    app->Init();
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


BASECODEDLL_API BCLayers* __stdcall BCExtractLayers(void *context, BCBitmapInfo bitmap, const double* palette, int paletteSize, const char *constraints, const bool autoCorrect, const char* imageFile)
{
	if (context == NULL) return 0;
	App *app = (App*) context;

	Vector<Vec3f> p;
	int numColors = paletteSize;
	double* data = (double*)palette;

	for (int i=0; i<numColors; i++)
		p.PushEnd(Vec3f((float)data[3*i], (float)data[3*i+1], (float)data[3*i+2]));
	
	return app->ExtractLayers(bitmap, p, constraints, autoCorrect, imageFile);
}

BASECODEDLL_API BCBitmapInfo* __stdcall BCSegmentImage(void* context, BCBitmapInfo bitmap)
{
	if (context == NULL) return 0;
	App *app = (App*) context;
	

	return app->SegmentImage(bitmap);

}

BASECODEDLL_API BCLayers* __stdcall BCSynthesizeLayers(void* context)
{
	if (context == NULL) return 0;
	App *app = (App*) context;

	return app->SynthesizeLayers();
}

BASECODEDLL_API void BCOutputMesh(void* context, BCBitmapInfo bitmap, const double* palette, int paletteSize, const char* filename)
{
	if (context == NULL) return;
	App  *app = (App*)context;

	Vector<Vec3f> p;
	int numColors = paletteSize;
	double* data = (double*)palette;

	for (int i=0; i<numColors; i++)
		p.PushEnd(Vec3f((float)data[3*i], (float)data[3*i+1], (float)data[3*i+2]));
	
	app->OutputMesh(bitmap, p, filename);

}

BASECODEDLL_API void BCGetWords(void* context, const char* filename)
{
	if (context == NULL) return;
	App  *app = (App*)context;

	app->GetWords(filename);
}

BASECODEDLL_API void BCLoadVideo(void* context, const char* filename, int paletteSize)
{
	if (context == NULL) return;
	App *app = (App*) context;

	app->LoadVideo(filename, paletteSize);
}

BASECODEDLL_API int BCGetVideoPaletteSize(void* context)
{
	if (context == NULL) return 0;
	App *app = (App*) context;

	return app->GetVideoPaletteSize();
}

BASECODEDLL_API byte BCGetVideoPalette(void* context, int paletteindex, int index)
{
	if (context == NULL) return 0;
	App *app = (App*) context;

	return app->GetVideoPalette(paletteindex, index);
}

BASECODEDLL_API void BCSetVideoPalette(void* context, int paletteindex, byte r, byte g, byte b)
{
	if (context == NULL) return;
	App *app = (App*) context;

	app->SetVideoPalette(paletteindex, r, g, b);
}

BASECODEDLL_API byte BCGetOriginalVideoPalette(void* context, int paletteindex, int index)
{
	if (context == NULL) return 0;
	App *app = (App*) context;

	return app->GetOriginalVideoPalette(paletteindex, index);
}

BASECODEDLL_API void BCSaveVideoFrames(void* context)
{
	if (context == NULL) return;
	App *app = (App*) context;

	app->SaveVideoFrames();
}

BASECODEDLL_API void BCSetVideoPreviewLayerIndex( void* context, int index )
{
	if (context == NULL) return;
	App *app = (App*) context;

	app->SetVideoPreviewLayerIndex(index);
}

BASECODEDLL_API void BCSaveVideoPaletteImage(void* context)
{
	if (context == NULL) return;
	App *app = (App*) context;

	app->saveVideoPaletteImage();
}

BASECODEDLL_API int __stdcall BCGetVideoHeight(void *context)
{
    if(context == NULL) return 0;
    App &app = *(App*)context;
	return app.GetVideoHeight();
}
BASECODEDLL_API int __stdcall BCGetVideoWidth(void *context)
{
    if(context == NULL) return 0;
    App &app = *(App*)context;
	return app.GetVideoWidth();
}