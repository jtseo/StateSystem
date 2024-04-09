#pragma once

#ifdef GDIPLUSLIB_EXPORTS
#define GDIPLUSLIB_API __declspec(dllexport)
#else
#define GDIPLUSLIB_API __declspec(dllimport)
#endif

extern "C" GDIPLUSLIB_API bool CreateAndSaveImage(const char* framePath, const char *picturePath, int _w, int _h);

