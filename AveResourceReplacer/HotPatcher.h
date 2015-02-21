#pragma once
#include <windef.h>

BOOL PatchFunctionInDll(const WCHAR* dll, const char* functionName, void* newAddress, void** ptrToOriginalFunction);
BOOL UnpatchFunctionInDll(const WCHAR* dll, const char* functionName, void* patchedFunction);