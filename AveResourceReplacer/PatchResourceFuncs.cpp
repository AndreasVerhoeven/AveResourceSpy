#include "stdafx.h"
#include "PatchResourceFuncs.h"
#include "Hotpatcher.h"
#include "ResourceSpy.h"
#include "Util.h"

#include "ResourceReplacerFuncs.h"
#include <psapi.h>

#include <map>
#include <vector>
#include <string>

CRITICAL_SECTION loadedResourcesCs = {0};
BOOL didInit = FALSE;

std::map<AveLoadedResource*, AveLoadedResource*> loadedResources;
std::map<HGLOBAL, AveLoadedResource*> fullyLoadedResources;
std::map<std::wstring, AveLoadedResource*> loadedResourcesByFile;

HRSRC (WINAPI *OriginalFindResourceExW)(HMODULE hModule,LPCWSTR lpType,LPCWSTR lpName,WORD    wLanguage);
HGLOBAL (WINAPI *OriginalLoadResource)(HMODULE hModule,HRSRC hResInfo) = NULL;
BOOL (WINAPI *OriginalFreeResource)(HGLOBAL hResData) = NULL;
DWORD (WINAPI *OriginalSizeofResource)(HMODULE hModule,HRSRC hResInfo) = NULL;


HRSRC WINAPI AveFindResourceExW(HMODULE hModule,LPCWSTR lpType,LPCWSTR lpName,WORD    wLanguage);
HGLOBAL WINAPI AveLoadResource(HMODULE hModule,HRSRC hResInfo);
BOOL WINAPI AveFreeResource(HGLOBAL hResData);
DWORD WINAPI AveSizeofResource(HMODULE hModule,HRSRC hResInfo);

void UnloadAllAveResources();

void InitResourceFuncs()
{
	if(!didInit)
	{
		InitializeCriticalSection(&loadedResourcesCs);
		didInit = TRUE;
	}
}

void UninitResourceFuncs()
{
	if(didInit)
	{
		UnloadAllAveResources();
		DeleteCriticalSection(&loadedResourcesCs);

		didInit = FALSE;
	}
	
}

void PatchResourceFuncs()
{
	InitResourceFuncs();

	if(OriginalFindResourceExW == NULL) PatchFunctionInDll(L"kernel32.dll", "FindResourceExW", &AveFindResourceExW, (void**)&OriginalFindResourceExW);
	if(OriginalLoadResource == NULL) PatchFunctionInDll(L"kernel32.dll", "LoadResource", &AveLoadResource, (void**)&OriginalLoadResource);
	if(OriginalFreeResource == NULL) PatchFunctionInDll(L"kernel32.dll", "FreeResource", &AveFreeResource, (void**)&OriginalFreeResource);
	if(OriginalSizeofResource == NULL) PatchFunctionInDll(L"kernel32.dll", "SizeofResource", &AveSizeofResource, (void**)&OriginalSizeofResource);
}

void UnpatchResourceFuncs()
{
	if(OriginalFindResourceExW != NULL)
	{
		UnpatchFunctionInDll(L"kernel32.dll", "FindResourceExW", AveFindResourceExW);
		OriginalFindResourceExW = NULL;
	}

	if(OriginalLoadResource != NULL)
	{
		UnpatchFunctionInDll(L"kernel32.dll", "LoadResource", AveLoadResource);
		OriginalLoadResource = NULL;
	}

	if(OriginalFreeResource != NULL)
	{
		UnpatchFunctionInDll(L"kernel32.dll", "FreeResource", AveFreeResource);
		OriginalFreeResource = NULL;
	}

	if(OriginalSizeofResource != NULL)
	{
		UnpatchFunctionInDll(L"kernel32.dll", "SizeofResource", AveSizeofResource);
		OriginalSizeofResource = NULL;
	}

	UninitResourceFuncs();
}

WCHAR* ResourceTypeIdToString(DWORD id)
{
	switch(id)
	{
		case RT_CURSOR: return L"CURSOR";
		case RT_BITMAP: return L"BITMAP";
		case RT_ICON: return L"ICON";
		case RT_MENU: return L"MENU";
		case RT_DIALOG: return L"DIALOG";
		case RT_STRING: return L"STRING";
		case RT_FONTDIR: return L"FONTDIR";
		case RT_FONT: return L"FONT";
		case RT_ACCELERATOR: return L"ACCELERATOR";
		case RT_RCDATA: return L"RCDATA";
		case RT_MESSAGETABLE: return L"MESSAGETABLE";
		
		case RT_GROUP_CURSOR: return L"GROUP_CURSOR";
		case RT_GROUP_ICON: return L"GROUP_ICON";
		case RT_VERSION: return L"VERSION";
		case RT_DLGINCLUDE: return L"DLGINCLUDE";
		case RT_PLUGPLAY: return L"PLUGPLAY";
		case RT_VXD: return L"VXD";
		case RT_ANICURSOR: return L"ANICURSOR";
		case RT_ANIICON: return L"ANIICON";
		case RT_HTML: return L"HTML";
		case RT_MANIFEST: return L"MANIFEST";	
	}

	return NULL;
}



BOOL GetOverridenResourcePath(HMODULE hInst, LPCWSTR lpType, LPCWSTR lpName, WCHAR* overridenPath, SIZE_T overridenPathLength)
{
	WCHAR modulePath[MAX_PATH] = {0};
	if(!GetModulePath(hInst, modulePath, _countof(modulePath)))
		return FALSE;


	WCHAR typeString[255] = {0};
	WCHAR nameString[255] = {0};

	if(IS_INTRESOURCE(lpType))
	{
		WCHAR* prettyName = ResourceTypeIdToString((DWORD)lpType);
		if(prettyName != NULL)
			wcscpy_s(typeString, _countof(typeString), prettyName);
		else
			_itow_s((DWORD)lpType, typeString, _countof(typeString), 10);
	}
	else if(lpType != NULL && wcslen(lpType) < _countof(typeString))
			wcscpy_s(typeString, _countof(typeString), lpType);

	if(IS_INTRESOURCE(lpName))
		_itow_s((DWORD)lpName, nameString, _countof(nameString), 10);
	else if(lpName != NULL && wcslen(lpName) < _countof(nameString))
			wcscpy_s(nameString, _countof(nameString), lpName);


	WCHAR searchPath[MAX_PATH] = {0};
	GetResourceLookupBasePath(searchPath);

	WCHAR searchPathSuffix[MAX_PATH] = {0};
	PathAppend(searchPathSuffix, PathFindFileName(modulePath));
	PathAppend(searchPathSuffix, typeString);
	PathAppend(searchPathSuffix, nameString);
	wcscat_s(searchPathSuffix, _countof(searchPathSuffix), L".bin");

	
	WCHAR thisApp[MAX_PATH] = {0};
	GetModuleFileName(NULL, thisApp, _countof(thisApp));

	WCHAR searchPathLocal[MAX_PATH] = {0};
	wcscpy_s(searchPathLocal, _countof(searchPathLocal), searchPath);
	PathAppend(searchPathLocal, PathFindFileName(thisApp)); 
	PathAppend(searchPathLocal, searchPathSuffix);
	if(PathFileExists(searchPathLocal))
	{
		wcscpy_s(overridenPath, overridenPathLength, searchPathLocal);
		return TRUE;
	}
	else
	{
		PathAppend(searchPath, L"all");
		PathAppend(searchPath, searchPathSuffix);
		if(PathFileExists(searchPath))
		{
			wcscpy_s(overridenPath, overridenPathLength, searchPath);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}

AveLoadedResource* AvePreloadResource(HMODULE hModule, LPCWSTR lpType, LPCWSTR lpName)
{	
	WCHAR filePath[MAX_PATH] = {0};
	BOOL hasCustomResource = GetOverridenResourcePath(hModule, lpType, lpName, filePath, _countof(filePath));
	

	NotifySpy(hModule, L"FindResourceExW", lpType, lpName, NULL, 0, hasCustomResource? filePath : L"");

	if(!hasCustomResource)
		return NULL;

	_wcslwr_s(filePath, _countof(filePath));

	EnterCriticalSection(&loadedResourcesCs);

	std::map<std::wstring, AveLoadedResource*>::iterator iter = loadedResourcesByFile.find(filePath);
	if(iter != loadedResourcesByFile.end())
	{
		LeaveCriticalSection(&loadedResourcesCs);
		return iter->second;
	}

	HANDLE  hFile = CreateFile(filePath, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, 0, NULL);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		LeaveCriticalSection(&loadedResourcesCs);
		return NULL;
	}

	AveLoadedResource* loadedResource = new AveLoadedResource;
	memset(loadedResource, 0, sizeof(AveLoadedResource));
	wcscpy_s(loadedResource->filePath, _countof(loadedResource->filePath), filePath);

	loadedResource->hModule = hModule;
	GetModulePath(hModule, loadedResource->modulePath, _countof(loadedResource->modulePath));

	if(IS_INTRESOURCE(lpType))
		loadedResource->resTypeId = (DWORD)lpType;
	else if(lpType != NULL && wcslen(lpType) < _countof(loadedResource->resTypeName))
			wcscpy_s(loadedResource->resTypeName, _countof(loadedResource->resTypeName), lpType);

	if(IS_INTRESOURCE(lpName))
		loadedResource->resNameId = (DWORD)lpName;
	else if(lpName != NULL && wcslen(lpName) < _countof(loadedResource->resNameName))
			wcscpy_s(loadedResource->resNameName, _countof(loadedResource->resNameName), lpName);

	loadedResource->fileSize = GetFileSize(hFile, NULL);
	loadedResource->memory = NULL;
	loadedResource->refCount = 1;

	loadedResources.insert(std::make_pair(loadedResource,loadedResource));
	loadedResourcesByFile.insert(std::make_pair(filePath, loadedResource));

	LeaveCriticalSection(&loadedResourcesCs);

	CloseHandle(hFile);

	return loadedResource;
}

HGLOBAL AveEnsureFullyLoadedResource(AveLoadedResource* loadedResource)
{
	EnterCriticalSection(&loadedResourcesCs);

	if(NULL == loadedResource->memory)
	{
		HANDLE  hFile = CreateFile(loadedResource->filePath, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, 0, NULL);
		if(INVALID_HANDLE_VALUE == hFile)
			return NULL;

		loadedResource->memory = new BYTE[loadedResource->fileSize];
		DWORD numRead = 0;
		const DWORD BUFFER_READ_SIZE = 4096;
		while(numRead < loadedResource->fileSize)
		{
			DWORD numToRead = min(BUFFER_READ_SIZE, loadedResource->fileSize - numRead);
			DWORD numReadInThisPass = 0;
			if(!ReadFile(hFile, ((BYTE*)loadedResource->memory)+numRead, numToRead, &numReadInThisPass, NULL))
			{
				CloseHandle(hFile);
				delete[] loadedResource->memory;
				LeaveCriticalSection(&loadedResourcesCs);
				return NULL;
			}

			numRead += numReadInThisPass;
		}

		CloseHandle(hFile);

		loadedResource->refCount++;

		
		fullyLoadedResources.insert(std::make_pair(loadedResource->memory, loadedResource));
		LeaveCriticalSection(&loadedResourcesCs);

		return loadedResource->memory;
	}
	else
	{
		LeaveCriticalSection(&loadedResourcesCs);
		return loadedResource->memory;
	}
}

void AveFreeLoadedResource(AveLoadedResource* loadedResource, BOOL force=FALSE)
{
	EnterCriticalSection(&loadedResourcesCs);
	loadedResource->refCount--;
	if(loadedResource->refCount <= 1 || force) // if we have a refcount of 1, the data is only preloaded
	{
		fullyLoadedResources.erase(fullyLoadedResources.find(loadedResource->memory));
		delete[] loadedResource->memory;
		loadedResource->memory = NULL;
	}

	if(loadedResource->refCount < 1 || force)
	{
		loadedResources.erase(loadedResources.find(loadedResource));
		loadedResourcesByFile.erase(loadedResource->filePath);
		delete loadedResource;
	}
	LeaveCriticalSection(&loadedResourcesCs);

}


void UnloadAllAveResourcesForDll(const WCHAR* dll)
{
	EnterCriticalSection(&loadedResourcesCs);

	std::vector<AveLoadedResource*> toRemove;

	for(std::map<std::wstring, AveLoadedResource*>::iterator iter = loadedResourcesByFile.begin();
		iter != loadedResourcesByFile.end();
		++iter)
	{
		if(NULL == dll || _wcsicmp(dll, iter->second->filePath) == 0)
		{
			toRemove.push_back(iter->second);	
		}
	}

	for(std::vector<AveLoadedResource*>::iterator iter = toRemove.begin();
		iter != toRemove.end();
		++iter)
	{
		AveFreeLoadedResource(*iter);
	}

	LeaveCriticalSection(&loadedResourcesCs);
}

void UnloadAllAveResources()
{
	UnloadAllAveResourcesForDll(NULL);
}


AveLoadedResource* FindAveFullyLoadedResource(HGLOBAL hGlobal)
{
	std::map<HGLOBAL, AveLoadedResource*>::iterator iter = fullyLoadedResources.find(hGlobal);
	if(iter != fullyLoadedResources.end())
	{
		return iter->second;
	}
	else
	{
		return NULL;
	}
}

BOOL IsAveLoadedResource(HRSRC res)
{
	AveLoadedResource* key = (AveLoadedResource*)res;
	EnterCriticalSection(&loadedResourcesCs);
	BOOL found = loadedResources.find(key) != loadedResources.end();
	LeaveCriticalSection(&loadedResourcesCs);

	return found;
}

DWORD AveGetSizeOfResource(AveLoadedResource* loadedResource)
{
	return loadedResource->fileSize;
}



HRSRC WINAPI AveFindResourceExW(HMODULE hModule,LPCWSTR lpType,LPCWSTR lpName,WORD    wLanguage)
{
	
	HRSRC res = (HRSRC)AvePreloadResource(hModule, lpType, lpName);
	if(res != NULL)
		return res;

	return OriginalFindResourceExW(hModule, lpType, lpName, wLanguage);
}


HGLOBAL WINAPI AveLoadResource(HMODULE hModule,HRSRC hResInfo)
{
	if(IsAveLoadedResource(hResInfo))
	{
		return AveEnsureFullyLoadedResource((AveLoadedResource*)hResInfo);
	}

	return OriginalLoadResource(hModule, hResInfo);
}


BOOL WINAPI AveFreeResource(HGLOBAL hResData)
{
	AveLoadedResource* loadedResource = FindAveFullyLoadedResource(hResData);
	if(loadedResource != NULL)
	{
		AveFreeLoadedResource(loadedResource);
		return TRUE;
	}

	return OriginalFreeResource(hResData);
}


DWORD WINAPI AveSizeofResource(HMODULE hModule,HRSRC hResInfo)
{
	if(IsAveLoadedResource(hResInfo))
	{
		return AveGetSizeOfResource((AveLoadedResource*)hResInfo);
	}

	return OriginalSizeofResource(hModule, hResInfo);
}