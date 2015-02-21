#include "stdafx.h"
#include "ResourceReplacerFuncs.h"



#pragma data_seg(".AVESHELLHOOK")
WCHAR resourceLookupBasePath[MAX_PATH] = {0};
#pragma data_seg()
#pragma comment(linker, "/section:.AVESHELLHOOK,rws")


// ===================== GDI+ STUFF =====================

static BOOL didInitGdiplus = FALSE;

static GdiplusStartupInput gdiplusStartupInput;
static ULONG_PTR pGdiToken = 0;

void initGdiplus()
{
	didInitGdiplus = TRUE;

	GdiplusStartup(&pGdiToken,&gdiplusStartupInput,NULL); 
}

void uninitGdiplus()
{
	GdiplusShutdown(pGdiToken);
	didInitGdiplus = FALSE;
}

void ensureGdiplus()
{
	if(!didInitGdiplus)
		initGdiplus();
}

BOOL __stdcall SetResourceLookupBasePath(const WCHAR* path)
{
	if(NULL == path)
		return FALSE;

	wcscpy_s(resourceLookupBasePath, _countof(resourceLookupBasePath), path);

	return TRUE;
}

BOOL __stdcall GetResourceLookupBasePath(WCHAR* path)
{
	if(NULL == path)
		return FALSE;

	if(wcslen(resourceLookupBasePath) == 0)
	{
		SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, path);
		PathAppend(path, L"AveResources");
	}
	else
	{
		wcscpy_s(path, MAX_PATH, resourceLookupBasePath);
	}

	return TRUE;
}

HBITMAP AveLoadBitmap(const WCHAR* filename)
{
	ensureGdiplus();

	Bitmap source(filename);
	HBITMAP hBmp = NULL;
	source.GetHBITMAP(NULL, &hBmp);
	return hBmp;
}


BOOL GetOverrideImageResourcePath(HINSTANCE hInst, LPCWSTR name, UINT type, WCHAR* outputPath)
{
	if(NULL == outputPath)
		return FALSE;

	WCHAR moduleName[MAX_PATH] = {0};
	if(NULL == hInst)
	{
		wcscpy_s(moduleName, _countof(moduleName), L"_system");
	}
	else
	{
		WCHAR handlePath[MAX_PATH] = {0};
		GetModuleFileName(hInst, handlePath, MAX_PATH);
		WCHAR* handleFileName = PathFindFileName(handlePath);

		wcscpy_s(moduleName, _countof(moduleName), handleFileName);
	}

	WCHAR searchPathSuffix[MAX_PATH*3] = {0};
	wcscpy_s(searchPathSuffix, _countof(searchPathSuffix), moduleName);

	if(IS_INTRESOURCE(name))
	{
		WORD id = (WORD)(name);
		WCHAR buf[100] = {0};
		_itow_s(id, buf, _countof(buf), 10);
		PathAppend(searchPathSuffix, buf);
	}
	else
	{
		PathAppend(searchPathSuffix, name);
	}

	if(type == IMAGE_BITMAP)	wcscat_s(searchPathSuffix, _countof(searchPathSuffix), L".png");
	else if(type == IMAGE_ICON)	wcscat_s(searchPathSuffix, _countof(searchPathSuffix), L".ico");
	else if(type == IMAGE_CURSOR)wcscat_s(searchPathSuffix, _countof(searchPathSuffix), L".cur");

	WCHAR searchPath[MAX_PATH] = {0};
	GetResourceLookupBasePath(searchPath);
	if(wcslen(searchPath) == 0)
	{
		wcscpy_s(searchPath, _countof(searchPath), L"c:\\replace");
	}

	WCHAR thisApp[MAX_PATH] = {0};
	GetModuleFileName(NULL, thisApp, _countof(thisApp));

	WCHAR searchPathLocal[MAX_PATH] = {0};
	wcscpy_s(searchPathLocal, _countof(searchPathLocal), searchPath);

	PathAppend(searchPathLocal, PathFindFileName(thisApp)); 
	PathAppend(searchPathLocal, searchPathSuffix);

	if(PathFileExists(searchPathLocal))
	{
		wcscpy_s(outputPath, MAX_PATH, searchPathLocal);
		return TRUE;
	}
	else
	{
		PathAppend(searchPath, L"all");
		PathAppend(searchPath, searchPathSuffix);
		if(PathFileExists(searchPath))
		{
			wcscpy_s(outputPath, MAX_PATH, searchPath);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}