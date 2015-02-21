// AveResourceReplacer.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "PatchedFunctions.h"
#include "ExceptionList.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	if(DLL_PROCESS_ATTACH == ul_reason_for_call)
	{
		//WCHAR name[MAX_PATH] = {0};
		//GetModuleFileName(NULL, name, _countof(name));
		//WritePrivateProfileString(L"tmp", name, L"1", L"c:\\users\\ave\\desktop\\bla.ini");

		if(IsInExceptionApp())
			return FALSE;

		PatchAll();
	}
	else if(DLL_PROCESS_DETACH == ul_reason_for_call)
	{
		UnpatchAll();
	}

    return TRUE;
}
