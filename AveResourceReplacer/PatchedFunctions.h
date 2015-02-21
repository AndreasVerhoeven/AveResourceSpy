#pragma once
#include <windows.h>

void __stdcall PatchAll();
void __stdcall UnpatchAll();


#define PATCH_USER32_FUNCTION(name) PatchFunctionInDll(L"user32.dll", #name, &Ave##name, reinterpret_cast<void**>(&Original##name))
#define UNPATCH_USER32_FUNCTION(name) \
if(Original##name) \
{\
	UnpatchFunctionInDll(L"user32.dll", #name, Ave##name); \
	Original##name = NULL;\
}


HANDLE __stdcall CallOriginalLoadImageW(HINSTANCE hinst,LPCWSTR lpszName,UINT uType,int cxDesired,int cyDesired,UINT fuLoad);
