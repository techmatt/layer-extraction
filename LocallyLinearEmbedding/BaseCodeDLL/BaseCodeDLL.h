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
BASECODEDLL_API BCLayers*     __stdcall BCExtractLayers(void* context, BCBitmapInfo image, const double* palette, int paletteSize, const char *constraints, const bool autoCorrect, const char* imagefile);
BASECODEDLL_API BCBitmapInfo* __stdcall BCSegmentImage(void* context, BCBitmapInfo image);
BASECODEDLL_API BCLayers*	  __stdcall BCSynthesizeLayers(void* context);
BASECODEDLL_API void		  __stdcall BCOutputMesh(void* context, BCBitmapInfo image, const double* palette, int paletteSize, const char* filename);
BASECODEDLL_API void		  __stdcall BCLoadVideo(void* context, const char* filename, int paletteSize);
BASECODEDLL_API int           __stdcall BCGetVideoPaletteSize(void* context);
BASECODEDLL_API byte          __stdcall BCGetVideoPalette(void* context, int paletteindex, int index);
BASECODEDLL_API void		  __stdcall BCSetVideoPalette(void* context, int paletteindex, byte r, byte g, byte b);
BASECODEDLL_API byte          __stdcall BCGetOriginalVideoPalette(void* context, int paletteindex, int index);
BASECODEDLL_API void		  __stdcall BCSaveVideoFrames(void* context);
BASECODEDLL_API void		  __stdcall BCSaveVideoPaletteImage(void* context);
BASECODEDLL_API void		  __stdcall BCSetVideoPreviewLayerIndex(void* context, int index);
BASECODEDLL_API int           __stdcall BCGetVideoHeight(void *context);
BASECODEDLL_API int           __stdcall BCGetVideoWidth(void *context);
BASECODEDLL_API int           __stdcall BCLoadSuggestedRecolorings(void *context);
BASECODEDLL_API void          __stdcall BCLoadSuggestion(void *context, int index);
BASECODEDLL_API byte          __stdcall BCGetSuggestPalette(void *context, int index, int paletteindex, int channel);