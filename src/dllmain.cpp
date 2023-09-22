#include "pch.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH: {
        Plugin.Init(hModule);
        break;
    }
    case DLL_PROCESS_DETACH: {
        Plugin.Shutdown();
    }
    }
    return TRUE;
}