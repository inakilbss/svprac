#include "pch.h"

#include <codecvt>
#include <format>
#include <locale>
#include <vector>

import StellaHook;

int popupError(std::string userMessage)
{
    return MessageBoxA(NULL, (userMessage + "\nQuit program?").c_str(), "Hook Error", MB_YESNO);
}

void popupError(std::wstring userMessage)
{
    DWORD errorCode = GetLastError();
	#pragma warning(suppress : 4996) // STL encoding conversion is deprecated but using Win32 for string processing is unreasonably hard
    std::wstring errorMessage = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().from_bytes(std::system_category().message(errorCode));
    std::wstring fullMessage = std::format(L"{} (code {}):\n{}", userMessage, errorCode, errorMessage).c_str();
    MessageBoxW(NULL, fullMessage.c_str(), userMessage.c_str(), MB_OK);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
			BasicHook::setTargetBase(GetModuleHandleW(NULL));
    		//BasicHook testHook(0x6eeb64, "48415800");
            std::vector<NopHook> rankExHooks({
                NopHook(0x1094ad, 18), // hidden boss
				NopHook(0x101f3e, 13), // rings type C
				NopHook(0x103933, 13), // rings type S
                NopHook(0x31dc3b, 9), // angry tetrapedes
                NopHook(0x31dece, 9), // 3 way poniards
                NopHook(0x31df1b, 9), // 3 way poniards not on ESY-NML

            });
            for (auto i = rankExHooks.begin(); i != rankExHooks.end(); ++i)
            {
                i->enable();
            }
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}