#pragma once
#include <windows.h>

#define MAX_FUNCTION_NAME 1024
#define MAX_RESOURCE_NAME 1024
#define MAX_RESOURCE_TYPE_NAME 1024
#define MAX_CUSTOM_DATA	256


struct AveResourceLoadNotification
{
	DWORD cbSize;							// the size of this structure, in bytes

	DWORD pid;								// process id of the process loading the resource
	WCHAR processPath[MAX_PATH];			// path of the process loading the resource
	WCHAR modulePath[MAX_PATH];				// the name of the resource module holding the resource that was requested

	WCHAR functionName[MAX_FUNCTION_NAME];	// the name of the function called


	WCHAR resourceTypeId;					// the id of the type of the resource being loaded, if no name is used
	WCHAR resourceTypeName[MAX_RESOURCE_TYPE_NAME]; // the name of the type of the resource being loaded

	WORD resourceId;						// the id of the resource being loaded, if no name is used
	WCHAR resourceName[MAX_RESOURCE_NAME];	// the name of the resource being loaded

	BYTE customData[MAX_CUSTOM_DATA];		// any custom data to be passed, depends on functionName

	WCHAR overridenResourcePath[MAX_PATH];	// the path of the overriden resource that would be loaded
};

BOOL NotifySpy(HINSTANCE hInst, const WCHAR* functionName, LPCWSTR resType, LPCWSTR resName, 
			   BYTE* customData, SIZE_T customDataLength,	WCHAR* overridenResourcePath);



BOOL __stdcall SetSpyCallbackWindow(HWND hwnd, UINT message);
HWND __stdcall GetSpyCallbackWindow(UINT* message);

