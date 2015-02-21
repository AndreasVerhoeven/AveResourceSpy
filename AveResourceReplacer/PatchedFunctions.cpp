#include "stdafx.h"
#include "PatchedFunctions.h"
#include "Hotpatcher.h"
#include "ResourceReplacerFuncs.h"
#include "ResourceSpy.h"
#include "PatchResourceFuncs.h"
#include "Util.h"

HANDLE (__stdcall *OriginalLoadImageW)(HINSTANCE, LPCWSTR, UINT, int, int, UINT) = NULL;
HANDLE __stdcall AveLoadImageW(HINSTANCE hinst,LPCWSTR lpszName,UINT uType,int cxDesired,int cyDesired,UINT fuLoad);



void __stdcall PatchAll()
{
	WCHAR currentProcessPath[MAX_PATH] = {0};
	GetModuleFileName(NULL, currentProcessPath, _countof(currentProcessPath));
	if(_wcsicmp(findFileName(currentProcessPath), L"ResourceSpy.exe") == 0)
		return;

	PATCH_USER32_FUNCTION(LoadImageW);

	PatchResourceFuncs();
}

void __stdcall UnpatchAll()
{
	UnpatchResourceFuncs();
	UNPATCH_USER32_FUNCTION(LoadImageW);
}



HANDLE __stdcall CallOriginalLoadImageW(HINSTANCE hinst,LPCWSTR lpszName,UINT uType,int cxDesired,int cyDesired,UINT fuLoad)
{
	if(OriginalLoadImageW != NULL)
		return OriginalLoadImageW(hinst, lpszName, uType, cxDesired, cyDesired, fuLoad);
	else
		return LoadImageW(hinst, lpszName, uType, cxDesired, cyDesired, fuLoad);
}

HANDLE __stdcall AveLoadImageW(HINSTANCE hInst,LPCWSTR lpszName,UINT type,int cxDesired,int cyDesired,UINT fuLoad)
{
	WCHAR overridenResourcePath[MAX_PATH] = {0};
	BOOL shouldOverride = GetOverrideImageResourcePath(hInst, lpszName, type, overridenResourcePath);

	//if(lpszName == MAKEINTRESOURCE(34562))
		//DebugBreak();

	LPCWSTR resType =	IMAGE_BITMAP == type ? RT_BITMAP :
						IMAGE_ICON   == type ? RT_ICON   :
						IMAGE_CURSOR == type ? RT_CURSOR : 0;
							
	NotifySpy(hInst, L"LoadImageW", resType, lpszName, NULL, 0, overridenResourcePath);

	if(shouldOverride)
	{
		if(IMAGE_BITMAP == type)
		{
			return AveLoadBitmap(overridenResourcePath);
		}
		else if(IMAGE_ICON == type)
		{
			return CallOriginalLoadImageW(NULL, overridenResourcePath, type, cxDesired, cyDesired, fuLoad | LR_LOADFROMFILE);
		}
		else if(IMAGE_CURSOR == type)
		{
			return CallOriginalLoadImageW(NULL, overridenResourcePath, type, cxDesired, cyDesired, fuLoad | LR_LOADFROMFILE);
		}
	}

	return CallOriginalLoadImageW(hInst, lpszName, type, cxDesired, cyDesired, fuLoad);
}