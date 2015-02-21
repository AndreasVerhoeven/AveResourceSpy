#pragma once
#include <windows.h>


struct AveLoadedResource
{
	HMODULE hModule;
	WCHAR   modulePath[MAX_PATH];

	DWORD   resTypeId;
	WCHAR   resTypeName[256];
	DWORD   resNameId;
	WCHAR	resNameName[256];

	WCHAR   filePath[MAX_PATH];
	DWORD   fileSize;
	LPVOID  memory;
	int     refCount;
};

void PatchResourceFuncs();
void UnpatchResourceFuncs();