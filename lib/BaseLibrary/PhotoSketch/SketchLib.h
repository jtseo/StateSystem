#pragma once

#ifdef GDIPLUSLIB_EXPORTS
#define GDIPLUSLIB_API __declspec(dllexport)
#else
#define GDIPLUSLIB_API __declspec(dllimport)
#endif

typedef bool (*fnSketchCallback)(int _step, HBITMAP _map, void *_param_p);
extern "C" GDIPLUSLIB_API bool CreateAndSaveImage(const char* framePath, const char *picturePath, int _w, int _h, int _sketchType, fnSketchCallback _callbck, void* _param_p);

