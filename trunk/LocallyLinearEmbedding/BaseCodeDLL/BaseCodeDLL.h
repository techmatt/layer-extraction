// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the BASECODEDLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// BASECODEDLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef BASECODEDLL_EXPORTS
#define BASECODEDLL_API __declspec(dllexport)
#else
#define BASECODEDLL_API __declspec(dllimport)
#endif

struct BCBitmapInfo
{
    UINT width;
    UINT height;
    BYTE *colorData;
};

struct BCLayerInfo
{
	UINT width;
	UINT height;

	double d0;
	double d1;
	double d2;

	double* weights;
};

struct BCLayers
{
	int numLayers;
	BCLayerInfo *layers;
};


BASECODEDLL_API void*         __stdcall BCInit();
BASECODEDLL_API UINT32        __stdcall BCProcessCommand(void *context, const char *s);
BASECODEDLL_API const char*   __stdcall BCQueryStringByName(void *context, const char *s);
BASECODEDLL_API int           __stdcall BCQueryIntegerByName(void *context, const char *s);
BASECODEDLL_API BCBitmapInfo* __stdcall BCQueryBitmapByName(void *context, const char *s);
BASECODEDLL_API BCLayers*     __stdcall BCExtractLayers(void* context, BCBitmapInfo image, const double* palette, int paletteSize, const char *constraints);
BASECODEDLL_API BCBitmapInfo* __stdcall BCSegmentImage(void* context, BCBitmapInfo image);