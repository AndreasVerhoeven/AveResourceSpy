#pragma once

BOOL __stdcall SetResourceLookupBasePath(const WCHAR* path);
BOOL __stdcall GetResourceLookupBasePath(WCHAR* path);
BOOL GetOverrideImageResourcePath(HINSTANCE hInst, LPCWSTR name, UINT uType, WCHAR* outputPath);

HBITMAP AveLoadBitmap(const WCHAR* filename);