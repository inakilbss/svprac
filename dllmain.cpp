// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "psapi.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        BYTE* targetBase = (BYTE*)GetModuleHandleW(NULL);
        BYTE* targetAddress = targetBase + 0x6eeb64;
        DWORD oldProtect;
        VirtualProtect(targetAddress, 4, PAGE_READWRITE, &oldProtect);
        *(DWORD*)targetAddress = 0x00584148;
        VirtualProtect(targetAddress, 4, oldProtect, &oldProtect);
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}