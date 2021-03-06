#include "stdafx.h"
#include "LOMNHook.h"
#include "LOMNAPI.h"

#include "MinHook.h"
#include "LOMNAPI\ScProcess.h"

#include "Mods/FrameThrottle.h"

char* WindowName = (char*)0x00732D84;

char GlobalInitFuncData[100 * 4] = { };

ScVector<unsigned long> ScGlobalInitFuncVector(100); // This constructor uses the new operator which produces garbage in DllMain.

ScVector<unsigned long>** ScGlobalInitFuncs = (ScVector<unsigned long>**)0x007201C0;

int* pRenderObjectIDMap = (int*)0x0079388C;

int* gProcessManager = (int*)0x00712D90;

char* useRealDeltaTime = (char*)0x00779920;
double* ScProcessManager__sFakeDeltaTime = (double*)0x00712D68;
double* ScProcessManager__sRealDeltaTime = (double*)0x00712D70;
bool* ScProcessManager__sUseReal = (bool*)0x00712D78;

typedef int (__thiscall *ScProcessManager__AddProcess)(void* pThis, ScProcess* item, unsigned char priority);
ScProcessManager__AddProcess pScProcessManager__AddProcess = nullptr; // Orig

typedef void(*ScProcessManager__UseRealDeltaTime)(bool useRealTime); // Type
ScProcessManager__UseRealDeltaTime pScProcessManager__UseRealDeltaTime = nullptr; // Original
ScProcessManager__UseRealDeltaTime tScProcessManager__UseRealDeltaTime = nullptr; // Trampouline

typedef void (__cdecl *GcGame__DoSetUp)(void);
GcGame__DoSetUp pGcGame__DoSetUp = nullptr;
GcGame__DoSetUp tGcGame__DoSetUp = nullptr;

void hScProcessManager__UseRealDeltaTime(bool useRealTime) {
	OutputDebugStringW(L"Setting UseRealDeltaTime to ");
	OutputDebugStringW(std::to_wstring((int)useRealTime).c_str());
	OutputDebugStringW(L"\n");
	tScProcessManager__UseRealDeltaTime(useRealTime);
}

void __cdecl hGcGame__DoSetUp() {
	OutputDebugStringW(L"Setting up game...\n");
	tGcGame__DoSetUp();
	OutputDebugStringW(L"Native setup complete...\n");
	//DebugBreak();

	// Try adding a new ScProcess (NOTE: Must happen after gProcessManager is created by native, which is after all the InitFuncs have run =(. DoSetUp is where the other subsystems add their processes, so we do that too.)
	FrameThrottle* throttle = new FrameThrottle();

	pScProcessManager__AddProcess(*(void**)gProcessManager, throttle, 1);
	OutputDebugStringW(L"Modded setup complete...\n");
}

void LOMNHook::Hook() {
	OutputDebugStringW(L"LOMNHook Hooking!\n");

	WindowName[3] = (char)0;

	// Add a custom InitFunc
	*ScGlobalInitFuncs = &ScGlobalInitFuncVector;
	(*ScGlobalInitFuncs)->Data = (unsigned long*)GlobalInitFuncData; // Overwrite pointer to the new operator gibberish with a pointer to some nice static memory
	(*ScGlobalInitFuncs)->PushBack((unsigned long)&LOMNHook::PreInit);

	// Load the native functions
	pScProcessManager__AddProcess = (ScProcessManager__AddProcess)0x004A8F50;
	pScProcessManager__UseRealDeltaTime = (ScProcessManager__UseRealDeltaTime)0x004A90E0;
	pGcGame__DoSetUp = (GcGame__DoSetUp)0x00436A10;

	// MinHook hooking
	if (MH_Initialize() != MH_OK)
	{
		OutputDebugStringW(L"LOMNHook: MinHook failed to init!");
	}
	else
	{
		MH_CreateHook(pScProcessManager__UseRealDeltaTime, hScProcessManager__UseRealDeltaTime, (void**)&tScProcessManager__UseRealDeltaTime);
		MH_CreateHook(pGcGame__DoSetUp, hGcGame__DoSetUp, (void**)&tGcGame__DoSetUp);
		MH_EnableHook(MH_ALL_HOOKS);
	}
}

void testCallback1() {
	OutputDebugStringW(L"test callback!");
}

void TestProcess(ScProcess* pthis, double time, double dt)
{
	OutputDebugStringW(L"Custom process func!!\n");
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

		//SetWindowTextW(GetWindowHandle(), L"beni's cool game!");
		//(pScProcessManager__UseRealDeltaTime)(true);
		//pAddFunc(gProcessManager, )
	}
	else {
		OutputDebugStringW(L"LOMNHook Post-Init Shutdown Inventory\n");

		for (int i = *(pRenderObjectIDMap + 2); i != *(pRenderObjectIDMap + 2) + *(pRenderObjectIDMap + 1) * 8; i += 8) {
			int id = *((int*)i + 1);
			OutputDebugStringW(std::to_wstring(id).c_str());
			OutputDebugStringW(L"\n");
		}

	}
}