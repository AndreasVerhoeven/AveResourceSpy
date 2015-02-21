#include "stdafx.h"
#include "ReplacedResourcesCache.h"
#include <map>

#pragma data_seg(".AVESHELLHOOK")
int globalCacheUpdateNumber=0;
#pragma data_seg()
#pragma comment(linker, "/section:.AVESHELLHOOK,rws")


CRITICAL_SECTION replacedResourcesCacheCs = {0};
BOOL didInit = FALSE;
int localCacheUpdateNumber=-1;

std::map<CString> existingReplacements;



void initReplacedResourcesCache()
{
	if(didInit)
		return;

	InitializeCriticalSection(&replacedResourcesCacheCs);
	didInit = TRUE;
}

void uninitReplacedResourcesCache()
{
	if(!didInit)
		return;

	DeleteCriticalSection(&replacedResourcesCacheCs);
	didInit = FALSE;
}

bool isReplacedResourcesCacheDirty()
{
	return globalCacheUpdateNumber != localCacheUpdateNumber;
}

void updateReplacedResourceCache()
{
	EnterCriticalSection(&replacedResourcesCacheCs);

	existingReplacements.clear();

	// walk recursively over each file/folder

	LeaveCriticalSection(&replacedResourcesCacheCs);

	// preload stuff
	localCacheUpdateNumber = globalCacheUpdateNumber;
}

bool doesResourceReplacementExists(const WCHAR* file)
{
	BOOL exists = TRUE;

	EnterCriticalSection(&replacedResourcesCacheCs);

	if(isReplacedResourcesCacheDirty())
	{
		updateReplacedResourceCache();
	}

	exists = existingReplacements.find(file) != existingReplacements.end();

	LeaveCriticalSection(&replacedResourcesCacheCs);

	return exists;
}