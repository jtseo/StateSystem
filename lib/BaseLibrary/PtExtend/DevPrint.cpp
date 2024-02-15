#include <Windows.h>
#include "DevPrint.h"

bool DevPrint(const char* _printer, int _mode, const char *filename)
{
	HBITMAP _hBlt = (HBITMAP)LoadImage(NULL, filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	HANDLE hPrinter;
	DEVMODE* pDevMode;
	DWORD dwNeeded, dwRet;

	LPSTR name = (LPSTR)_printer;

	OpenPrinter(name, &hPrinter, NULL);
	dwNeeded = DocumentProperties(NULL, hPrinter, name, NULL, NULL, 0);
	pDevMode = (DEVMODE*)GlobalAlloc(GMEM_FIXED, dwNeeded);

	dwRet = DocumentProperties(NULL, hPrinter, name, pDevMode, NULL, DM_OUT_BUFFER);
	if (dwRet != IDOK) {
		// Handle error
		return false;
	}
	/*
	// Assuming DNP printer uses standard paper size codes 165
	//pDevMode->dmPaperSize = 162;// 8 a3, 11 a5
	if(_mode == 0)
		pDevMode->dmPaperSize = 162;// 8 a3, 11 a5
	else
		pDevMode->dmPaperSize = 165;
	pDevMode->dmFields |= DM_PAPERSIZE;

	dwRet = DocumentProperties(NULL, hPrinter, name, pDevMode, pDevMode, DM_IN_BUFFER | DM_OUT_BUFFER);
	if (dwRet != IDOK) {
		// Handle error
	}
	//*/
	DOCINFO di = { sizeof(DOCINFO), TEXT("My Bitmap Print Job") };
	HDC printerDC = CreateDC(NULL, name, NULL, pDevMode); // Create a DC for the printer
	StartDoc(printerDC, &di);
	StartPage(printerDC);

	// Get the size of the bitmap
	BITMAP bitmap;
	GetObject(_hBlt, sizeof(bitmap), &bitmap);

	// Create a compatible DC for the bitmap
	HDC hdcMem = CreateCompatibleDC(printerDC);
	SelectObject(hdcMem, _hBlt);

	// Adjust the parameters as per your requirements
	BitBlt(printerDC, 0, 0, bitmap.bmWidth, bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);

	// Clean up
	DeleteDC(hdcMem);
	DeleteObject(_hBlt);

	EndPage(printerDC);
	EndDoc(printerDC);
	DeleteDC(printerDC);

	GlobalFree(pDevMode);
	ClosePrinter(hPrinter);

	return true;
}