#include "stdafx.h"
#include "ResourceSpy.h"
#include "Util.h"

#pragma data_seg(".AVESHELLHOOK")
HWND spyCallbackHwnd = NULL;
UINT spyCallbackMessage = 0;
#pragma data_seg()
#pragma comment(linker, "/section:.AVESHELLHOOK,rws")

#include "psapi.h"
#pragma comment(lib, "psapi.lib")

BOOL NotifySpy(HINSTANCE hInst, const WCHAR* functionName, LPCWSTR resType, LPCWSTR resName, 
			   BYTE* customData, SIZE_T customDataLength,	WCHAR* overridenResourcePath)
{
	if(!IsWindow(spyCallbackHwnd) || 0 == spyCallbackMessage)
		return FALSE;

	AveResourceLoadNotification notify = {sizeof(AveResourceLoadNotification)};

	notify.pid = GetCurrentProcessId();
	BOOL res = GetModuleFileName(NULL, notify.processPath, _countof(notify.processPath));
	

	if(hInst != NULL)
	{
		if((DWORD_PTR)hInst & 1)
			hInst -= 1;
		BOOL res = GetModuleFileName(hInst, notify.modulePath, _countof(notify.modulePath));
		if(!res || wcslen(notify.modulePath) == 0)
		{
			WCHAR devicePath[MAX_PATH*10] = {0};
			//HMODULE modHandle = GetModuleHandleFromAddress(hInst);
			// WEIRD HACK: the hmodule is sometimes actually pointing a couple of bytes
			//				BEFORE the loaded module
			if(GetMappedFileName(GetCurrentProcess(), hInst+20, devicePath, _countof(devicePath)) > 0)
			{
				WCHAR path[MAX_PATH] = {0};

				// GetMappedFileName() returns a device name, and we want a "win32" name
				if(!TranslateDevicePathToWin32Path(devicePath, path, _countof(path)))
					wcscpy_s(path, _countof(path), PathFindFileName(devicePath));

				wcscpy_s(notify.modulePath, _countof(notify.modulePath), path);
			}
			else
			{
				//DWORD lastError = GetLastError();
				//DebugBreak();
				//_itow_s((DWORD_PTR)hInst, notify.modulePath, _countof(notify.modulePath), 16);
			}
		}
	}
	else
	{
		wcscpy_s(notify.modulePath, _countof(notify.modulePath), L"<null>");
	}

	wcscpy_s(notify.functionName, _countof(notify.functionName), functionName);

	if(IS_INTRESOURCE(resType))
	{
		notify.resourceTypeId = (WORD)resType;
	}
	else
	{
		if(resType != NULL && wcslen(resType) < _countof(notify.resourceTypeName))
			wcscpy_s(notify.resourceTypeName, _countof(notify.resourceTypeName), resType);
	}


	if(IS_INTRESOURCE(resName))
	{
		notify.resourceId = (WORD)resName;
	}
	else
	{
		if(resName != NULL && wcslen(resName) < _countof(notify.resourceName))
			wcscpy_s(notify.resourceName, _countof(notify.resourceName), resName);
	}

	if(customData != NULL)
	{
		memcpy_s(notify.customData, _countof(notify.customData), customData, customDataLength);
	}

	if(overridenResourcePath != NULL)
		wcscpy_s(notify.overridenResourcePath, _countof(notify.overridenResourcePath), overridenResourcePath);


	COPYDATASTRUCT cds = {0};
	cds.dwData = spyCallbackMessage;
	cds.cbData = sizeof(notify);
	cds.lpData = &notify;

	SendMessage(spyCallbackHwnd, WM_COPYDATA, (WPARAM)NULL, (LPARAM)&cds);

	return TRUE;
}

BOOL __stdcall SetSpyCallbackWindow(HWND hwnd, UINT message)
{
	spyCallbackHwnd = hwnd;
	spyCallbackMessage = message;
	return TRUE;
}

HWND __stdcall GetSpyCallbackWindow(UINT* message)
{
	if(message != NULL)
		*message = spyCallbackMessage;

	return spyCallbackHwnd;
}
