#include "stdafx.h"
#include "ExceptionList.h"
#include "Util.h"


BOOL IsInExceptionApp()
{
	WCHAR path[MAX_PATH] = {0};
	GetModuleFileName(NULL, path, _countof(path));
	const WCHAR* appName = findFileName(path);

	WCHAR localAppDirExceptionsFile[MAX_PATH] = {0};
	GetEnvironmentVariable(L"LOCALAPPDATA", localAppDirExceptionsFile, _countof(localAppDirExceptionsFile));
	lstrcat(localAppDirExceptionsFile, L"\\AveResources\\exceptions.ini");

	return GetPrivateProfileInt(L"apps", appName, 0, localAppDirExceptionsFile) != 0;
}


BOOL IsExceptionModuleForApp(HMODULE hMod, const WCHAR* appName)
{
WCHAR path[MAX_PATH] = {0};
	GetModulePath(hMod, path, _countof(path));
	const WCHAR* moduleName = findFileName(path);

	WCHAR localAppDirExceptionsFile[MAX_PATH] = {0};
	GetEnvironmentVariable(L"LOCALAPPDATA", localAppDirExceptionsFile, _countof(localAppDirExceptionsFile));
	lstrcat(localAppDirExceptionsFile, L"\\AveResources\\exceptions.ini");

	return GetPrivateProfileInt(appName, moduleName, 0, localAppDirExceptionsFile) != 0;
}

BOOL IsExceptionModule(HMODULE hMod)
{
	WCHAR path[MAX_PATH] = {0};
	GetModuleFileName(NULL, path, _countof(path));
	const WCHAR* appName = findFileName(path);

	if(IsExceptionModuleForApp(hMod, appName) != 0)
		return TRUE;

	if(IsExceptionModuleForApp(hMod, L"all") != 0)
		return TRUE;

	return FALSE;
}
