#include "stdafx.h"

// ============= SHARED VARIABLES ===================
#pragma data_seg(".AVESHELLHOOK")
HHOOK hook = NULL;			// the hook's handle
#pragma data_seg()
#pragma comment(linker, "/section:.AVESHELLHOOK,rws")

LRESULT CALLBACK CallWndProc(int code, WPARAM wParam,  LPARAM lParam)
{
	return CallNextHookEx(hook, code, wParam, lParam);
}

// method to start the hook
BOOL CALLBACK StartHook(HMODULE hMod, HWND hwnd)
{
	if(hook != NULL)
		return FALSE;

	BOOL onlyExplorer = FALSE;

	WCHAR dllPath[MAX_PATH] = {0};
	GetModuleFileName(hMod, dllPath, MAX_PATH);
	PathRemoveFileSpec(dllPath);
	PathAppend(dllPath, L"exploreronly.txt");
	if(PathFileExists(dllPath))
	{
		onlyExplorer = TRUE;
	}

	if(onlyExplorer)
	{
		HWND tray = FindWindow(L"Shell_TrayWnd", NULL);
		if(NULL == tray)
			return FALSE;

		DWORD threadid = GetWindowThreadProcessId(tray, 0);
		hook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, hMod, threadid);		
	}
	else
	{
		hook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, hMod, 0);
	}

	return hook != NULL;
}

// method to stop the hook
BOOL CALLBACK StopHook()
{
	if(NULL == hook)
		return FALSE;

	BOOL res = UnhookWindowsHookEx(hook);
	if(res)
	{
		hook = NULL;
	}

	return res;
}

// returns TRUE iff the hook is running
BOOL CALLBACK IsHookRunning()
{
	return hook != NULL;
}
