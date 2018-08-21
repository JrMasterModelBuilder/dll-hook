#include "stdafx.h"
#include "LOMNHook.h"

#include "MinHook.h"
#include "Native/Vector.h"
#include "Native/ScProcess.h"

using namespace LOMNHook;

char GlobalInitFuncData[100 * 4] = { };

Native::Vector<unsigned long> ScGlobalInitFuncVector(100); // This constructor uses the new operator which produces garbage in DllMain.

Native::Vector<unsigned long>** ScGlobalInitFuncs = (Native::Vector<unsigned long>**)0x007201C0;

int* gProcessManager = (int*)0x00712D90;

typedef int (__thiscall *ScProcessManager__AddProcess)(void* pThis, Native::ScProcess* item, unsigned char priority);
ScProcessManager__AddProcess pScProcessManager__AddProcess = nullptr; // Orig

typedef void (__cdecl *GcGame__DoSetUp)(void);
GcGame__DoSetUp pGcGame__DoSetUp = nullptr;
GcGame__DoSetUp tGcGame__DoSetUp = nullptr;

void __cdecl hGcGame__DoSetUp() {
	OutputDebugStringW(L"Setting up game...\n");
	tGcGame__DoSetUp();
	OutputDebugStringW(L"Native setup complete...\n");
	//DebugBreak();

	OutputDebugStringW(L"Modded setup complete...\n");
}

void LOMNHook::Hook() {
	OutputDebugStringW(L"LOMNHook Hooking!\n");

	// Add a custom InitFunc
	*ScGlobalInitFuncs = &ScGlobalInitFuncVector;
	(*ScGlobalInitFuncs)->Data = (unsigned long*)GlobalInitFuncData; // Overwrite pointer to the new operator gibberish with a pointer to some nice static memory
	(*ScGlobalInitFuncs)->PushBack((unsigned long)&LOMNHook::PreInit);

	// Load the native functions
	pScProcessManager__AddProcess = (ScProcessManager__AddProcess)0x004A8F50;
	pGcGame__DoSetUp = (GcGame__DoSetUp)0x00436A10;

	// MinHook hooking
	if (MH_Initialize() != MH_OK)
	{
		OutputDebugStringW(L"LOMNHook: MinHook failed to init!");
	}
	else
	{
		MH_CreateHook(pGcGame__DoSetUp, hGcGame__DoSetUp, (void**)&tGcGame__DoSetUp);
		MH_EnableHook(MH_ALL_HOOKS);
	}
}

void testCallback1() {
	OutputDebugStringW(L"test callback!");
}

void LOMNHook::PreInit(bool shutDown) {
	if (!shutDown) {
		OutputDebugStringW(L"LOMNHook Initializing!\n");
		(*ScGlobalInitFuncs)->PushBack((unsigned long)&LOMNHook::PostInit);

	}
	else {
		OutputDebugStringW(L"LOMNHook Disposing!\n");
	}
}

void LOMNHook::PostInit(bool shutDown) {
	if (!shutDown) {
		OutputDebugStringW(L"LOMNHook Post-Init!\n");
	}
	else {
		OutputDebugStringW(L"LOMNHook Post-Init Shutdown Inventory\n");
	}
}