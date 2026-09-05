#include "pch.h"

#include <iostream>
#include <format>
#include <locale>

constexpr std::string targetFileName = "SV123.exe";
constexpr std::string dllFileName = "svprac.dll";
constexpr size_t dllFileNameSize = (dllFileName.size() + 1) * sizeof(wchar_t);
constexpr DWORD timeoutms = 10000;

static constexpr std::wstring toWide(const std::string &src) {
    std::wstring dst;
    for (const auto &c: src) {
        dst.push_back(c);
    }
    return dst;
}

static void popupSystemError(const std::string &userMessage) {
    DWORD dw = GetLastError();

    LPSTR lpMsgBuf;
    std::string systemMessage = FormatMessageA(
                                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                    FORMAT_MESSAGE_FROM_SYSTEM |
                                    FORMAT_MESSAGE_IGNORE_INSERTS,
                                    NULL,
                                    dw,
                                    MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                                    (LPSTR) &lpMsgBuf,
                                    0, NULL)
                                    ? std::format("Error code {}: {}", dw, lpMsgBuf)
                                    : std::format(
                                        "Error code {}: Check Windows docs for error details\nFormatting failed with error code {}",
                                        dw, GetLastError());
    LocalFree(lpMsgBuf);

    MessageBoxA(NULL, systemMessage.c_str(), userMessage.c_str(), MB_OK);

    ExitProcess(dw);
}

int main(int argc, char *argv[]) {
    PROCESS_INFORMATION procInfo;
    STARTUPINFOW startup_info = {};
    startup_info.cb = sizeof(startup_info);

    if (!CreateProcessW(toWide(targetFileName).c_str(), NULL, NULL, NULL, false, 0, NULL, NULL, &startup_info,
                        &procInfo)) {
        popupSystemError(std::format("Error creating process {}", targetFileName));
    } else {
        const HANDLE hProcess = procInfo.hProcess;
        const HANDLE hThread = procInfo.hThread;

        if (WaitForInputIdle(hProcess, timeoutms)) {
            popupSystemError("Error or timeout waiting for process initialization");
        } else if (SuspendThread(hThread) == -1) {
            popupSystemError("Error suspending main thread");
        } else {
            void *dllFileNameInTarget = VirtualAllocEx(hProcess, NULL, dllFileNameSize, MEM_COMMIT | MEM_RESERVE,
                                                       PAGE_READWRITE);
            if (dllFileNameInTarget == NULL) {
                popupSystemError("Error reserving memory for DLL name");
            } else {
                SIZE_T bytesWritten = 0;
                if (!WriteProcessMemory(hProcess, dllFileNameInTarget, toWide(dllFileName).c_str(), dllFileNameSize,
                                        &bytesWritten)) {
                    popupSystemError("Error writing DLL name to target");
                } else {
                    HANDLE dllThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE) LoadLibraryW,
                                                          dllFileNameInTarget, 0, NULL);
                    if (dllThread == NULL) {
                        popupSystemError("Error creating DLL initialization thread");
                    } else {
                        if (WaitForSingleObject(dllThread, timeoutms)) {
                            popupSystemError("Error or timeout waiting for DLL initialization");
                        } else if (DWORD moduleHandle; !GetExitCodeThread(dllThread, &moduleHandle) || !moduleHandle) {
                            popupSystemError("Potential error in DLL initialization");
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
