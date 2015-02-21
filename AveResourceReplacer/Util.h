#pragma once
#include <windows.h>

const WCHAR* findFileName(const WCHAR* path);
BOOL GetModulePath(HMODULE hInst, WCHAR* modulePath, SIZE_T modulePathLength);

BOOL GetModulePath(HMODULE hInst, WCHAR* modulePath, SIZE_T modulePathLength);
BOOL TranslateDevicePathToWin32Path(const WCHAR* path, WCHAR* output, SIZE_T maxOutputLen);