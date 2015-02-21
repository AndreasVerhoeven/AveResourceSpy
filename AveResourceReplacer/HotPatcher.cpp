#include "stdafx.h"

#include "HotPatcher.h"

inline void* AddToPtr(void* address, int numberOfBytesToAdd)
{
	BYTE* byteAddress = (BYTE*)address;
	byteAddress += numberOfBytesToAdd;
	return (void*)byteAddress;
}

#define OPCODE_NOP					0x90
#define OPCODE_SHORTJMP				0xEB
#define OPCODE_LONGJMP				0xE9
#define OPCODE_MOV					0x8B
#define OPCODE_MOV_PARAM_EDI_EDI	0xFF

#define USE_MHOOK

#ifdef USE_MHOOK//_M_X64
#include "mhook-lib/mhook.h"
#endif

BOOL PatchFunction(void* functionAddress, void* newAddress, void** ptrToOriginalFunction)
{
#ifdef USE_MHOOK//_M_X64
	BOOL bRes = Mhook_SetHook(&functionAddress, newAddress);
	if(ptrToOriginalFunction)
		*ptrToOriginalFunction = functionAddress;

	return bRes;
#else

	// before patching: NOP NOP NOP NOP NOP |function start| (MOV EDI, EDI) <real function>
	// after patching:  JMP newAddress		|function start| JMP -5         <real function>
	if(NULL == functionAddress || NULL == newAddress)
		return FALSE;

	BYTE* functionHotPatchHeader = (BYTE*)functionAddress;
	BYTE* functionProlog = (BYTE*)AddToPtr(functionAddress, -5);
	if(ptrToOriginalFunction != NULL)
		*ptrToOriginalFunction = AddToPtr(functionAddress, 2);

	bool isOriginal =	functionHotPatchHeader[0] == OPCODE_MOV && 
						functionHotPatchHeader[1] == OPCODE_MOV_PARAM_EDI_EDI &&
						functionProlog[0] == OPCODE_NOP && 
						functionProlog[1] == OPCODE_NOP &&
						functionProlog[2] == OPCODE_NOP &&
						functionProlog[3] == OPCODE_NOP &&
						functionProlog[4] == OPCODE_NOP;

	if(!isOriginal)
		return FALSE;

	DWORD oldProtect = 0;
	if(!VirtualProtect(functionProlog, 7, PAGE_EXECUTE_READWRITE, &oldProtect))
		return FALSE;

	// turn the prolog into a JMP newAddress
	functionProlog[0] = OPCODE_LONGJMP; // long jmp opcode
	LONG* longJmpOperand = (LONG*)AddToPtr(functionProlog, 1);
#pragma warning(push)
#pragma warning(disable: 4311)
	*longJmpOperand = (LONG)AddToPtr(newAddress, -(LONG)functionAddress);
#pragma warning(pop)

	functionHotPatchHeader[0] = OPCODE_SHORTJMP; // short relative jmp
	functionHotPatchHeader[1] = (BYTE)-7; // jump to the function prolog

	DWORD dummyOldProtect = 0;
	VirtualProtect(functionProlog, 7, oldProtect, &dummyOldProtect);

	FlushInstructionCache(GetCurrentProcess(), functionProlog, 7);

	return TRUE;
#endif
}

BOOL UnpatchFunction(void* functionAddress, void* patchedFunction)
{
#ifdef USE_MHOOK// _M_X64
	BOOL bRes = Mhook_Unhook(&patchedFunction);
	return bRes;
#else
	// before unpatching:   JMP newAddress		|function start| JMP -5         <real function>
	// after unpatching:	NOP NOP NOP NOP NOP |function start| (MOV EDI, EDI) <real function>
	
	if(NULL == functionAddress)
		return FALSE;

	BYTE* functionHotPatchHeader = (BYTE*)functionAddress;
	BYTE* functionProlog = (BYTE*)AddToPtr(functionAddress, -5);

	DWORD oldProtect = 0;
	if(!VirtualProtect(functionProlog, 7, PAGE_EXECUTE_READWRITE, &oldProtect))
		return FALSE;

	functionHotPatchHeader[0] = OPCODE_MOV;
	functionHotPatchHeader[1] = OPCODE_MOV_PARAM_EDI_EDI;
	functionProlog[0] = OPCODE_NOP;
	functionProlog[1] = OPCODE_NOP;
	functionProlog[2] = OPCODE_NOP;
	functionProlog[3] = OPCODE_NOP;
	functionProlog[4] = OPCODE_NOP;

	DWORD dummyOldProtect = 0;
	VirtualProtect(functionProlog, 7, oldProtect, &dummyOldProtect);

	FlushInstructionCache(GetCurrentProcess(), functionProlog, 7);

	return TRUE;
#endif
}

BOOL PatchFunctionInDll(const WCHAR* dll, const char* functionName, void* newAddress, void** ptrToOriginalFunction)
{
	HMODULE hMod = GetModuleHandle(dll);
	if(NULL == hMod)
		hMod = LoadLibrary(dll);

	if(NULL == hMod)
		return FALSE;

	void* functionAddress = GetProcAddress(hMod, functionName);
	if(NULL == functionAddress)
		return FALSE;

	return PatchFunction(functionAddress, newAddress, ptrToOriginalFunction);
}

BOOL UnpatchFunctionInDll(const WCHAR* dll, const char* functionName, void* patchedFunction)
{
	HMODULE hMod = GetModuleHandle(dll);
	if(NULL == hMod)
		hMod = LoadLibrary(dll);

	if(NULL == hMod)
		return FALSE;

	void* functionAddress = GetProcAddress(hMod, functionName);
	if(NULL == functionAddress)
		return FALSE;

	return UnpatchFunction(functionAddress, patchedFunction);
}
