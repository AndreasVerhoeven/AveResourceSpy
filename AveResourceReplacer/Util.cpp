#include "stdafx.h"
#include "Util.h"

#include "psapi.h"
#pragma comment(lib, "psapi.lib")

const WCHAR* findFileName(const WCHAR* path)
{
	const WCHAR* ptr = path;
	int len = lstrlen(path);
	ptr += len;

	while(ptr > path)
	{
		ptr -= 1;
		if(*ptr == L'/' || *ptr  == L'\\')
			return ptr + 1;
	}

	return ptr;
}

BOOL TranslateDevicePathToWin32Path(const WCHAR* path, WCHAR* output, SIZE_T maxOutputLen)
{
	if(NULL == output)
		return FALSE;

	WCHAR temp[512] = {0};
	if(GetLogicalDriveStrings(_countof(temp)-1, temp))
	{
		WCHAR name[MAX_PATH] = {0};
		WCHAR drive[3] = L" :";
		BOOL found = FALSE;
		WCHAR* p = temp;
		do
		{
			*drive = *p; // copy the drive to the drive string
			if(QueryDosDevice(drive, name, _countof(name)))
			{
				size_t nameLen = wcslen(name);
				if(nameLen < MAX_PATH)
				{
					found = _wcsnicmp(path, name, nameLen) == 0;
					if(found)
					{
						WCHAR tempFile[MAX_PATH] = {0};
						wcscpy_s(output, maxOutputLen, drive);
						wcscat_s(output, maxOutputLen, path+nameLen);
						
					}
				}
			}

			while(*p++);
		}
		while(!found && *p);

		return found;
	}
	else
	{
		return FALSE;
	}
}

BOOL GetModulePath(HMODULE hInst, WCHAR* modulePath, SIZE_T modulePathLength)
{
	if(hInst == NULL)
		hInst = GetModuleHandle(NULL);

	if(hInst != NULL)
	{
		if((DWORD_PTR)hInst & 1)
			hInst -= 1;
		BOOL res = GetModuleFileName(hInst, modulePath, (DWORD)modulePathLength);
		if(!res || wcslen(modulePath) == 0)
		{
			WCHAR devicePath[MAX_PATH*10] = {0};
			// WEIRD HACK: the hmodule is sometimes actually pointing a couple of bytes
			//				BEFORE the loaded module
			if(GetMappedFileName(GetCurrentProcess(), hInst+20, devicePath, _countof(devicePath)) > 0)
			{
				WCHAR path[MAX_PATH] = {0};

				// GetMappedFileName() returns a device name, and we want a "win32" name
				if(!TranslateDevicePathToWin32Path(devicePath, path, _countof(path)))
					wcscpy_s(path, _countof(path), PathFindFileName(devicePath));

				wcscpy_s(modulePath, modulePathLength, path);
			}
			else
			{
				//DWORD lastError = GetLastError();
				//DebugBreak();
				//_itow_s((DWORD_PTR)hInst, modulePath, modulePathLength, 16);
				return FALSE;
			}
		}
	}
	else
	{
		wcscpy_s(modulePath, modulePathLength, L"<null>");
	}

	return TRUE;
}