#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include "psapi.h"
#include <iostream>
#include <system_error>
#include <format>
#include <locale>
#include <codecvt>

const std::wstring targetFileName = L"SV123.exe";
const std::wstring dllFileName = L"svprac.dll";
const size_t dllFileNameSize = (dllFileName.size() + 1) * sizeof(wchar_t);
const DWORD timeoutms = 10000;

void popupSystemError(std::wstring userMessage) {
    DWORD errorCode = GetLastError();
    #pragma warning(suppress : 4996) // STL encoding conversion is deprecated but using Win32 for string processing is unreasonably hard
    std::wstring errorMessage = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().from_bytes(std::system_category().message(errorCode));
    std::wstring fullMessage = std::format(L"{} (code {}):\n{}", userMessage, errorCode, errorMessage).c_str();
    MessageBoxW(NULL, fullMessage.c_str(), userMessage.c_str(), MB_OK);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    PROCESS_INFORMATION procInfo;
    STARTUPINFOW startup_info;
    memset(&startup_info, 0, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    if (!CreateProcessW(targetFileName.c_str(), NULL, NULL, NULL, false, 0, NULL, NULL, &startup_info, &procInfo)) {
        popupSystemError(std::format(L"Error creating process {}", targetFileName));
    }
    else
    {
        HANDLE hProcess = procInfo.hProcess;
        HANDLE hThread = procInfo.hThread;

        if (WaitForInputIdle(hProcess, timeoutms)) {
            popupSystemError(L"Error or timeout waiting for process initialization");
        }
        else if (SuspendThread(hThread) == -1) {
            popupSystemError(L"Error suspending main thread");
        }
        else
        {
            void* dllFileNameInTarget = VirtualAllocEx(hProcess, NULL, dllFileNameSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
            if (dllFileNameInTarget == NULL) {
                popupSystemError(L"Error reserving memory for DLL name");
            }
            else
            {
                SIZE_T bytesWritten = 0;
                if (!WriteProcessMemory(hProcess, dllFileNameInTarget, dllFileName.c_str(), dllFileNameSize, &bytesWritten)) {
                    popupSystemError(L"Error writing DLL name to target");
                }
                else
                {
                    HANDLE dllThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryW, dllFileNameInTarget, 0, NULL);
                    if (dllThread == NULL) {
                        popupSystemError(L"Error creating DLL initialization thread");
                    }
                    else
                    {
                        if (WaitForSingleObject(dllThread, timeoutms)) {
                            popupSystemError(L"Error or timeout waiting for DLL initialization");
                        }
                        CloseHandle(dllThread);
                    }
                }
                VirtualFreeEx(hProcess, dllFileNameInTarget, 0, MEM_RELEASE);
            }
            ResumeThread(hThread);
        }
        CloseHandle(hThread);
        CloseHandle(hProcess);
    }
}