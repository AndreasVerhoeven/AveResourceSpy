#pragma once
#include <windows.h>

void initReplacedResourcesCache();
void uninitReplacedResourcesCache();
bool isReplacedResourcesCacheDirty();
void updateReplacedResourceCache();
bool doesResourceReplacementExists(const WCHAR* file);