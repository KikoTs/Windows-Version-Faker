#include "pch.h"
#include <Windows.h>
#include <winternl.h>
#include <detours.h>

// Define the function pointer types
typedef NTSTATUS(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW lpVersionInformation);
typedef DWORD(WINAPI* GetVersionPtr)();
typedef BOOL(WINAPI* GetVersionExPtr)(LPOSVERSIONINFO lpVersionInfo);
typedef BOOL(WINAPI* VerifyVersionInfoPtr)(LPOSVERSIONINFOEX lpVersionInfo, DWORD dwTypeMask, DWORDLONG dwlConditionMask);

RtlGetVersionPtr RealRtlGetVersion = (RtlGetVersionPtr)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "RtlGetVersion");
GetVersionPtr RealGetVersion = (GetVersionPtr)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "GetVersion");
GetVersionExPtr RealGetVersionEx = (GetVersionExPtr)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "GetVersionExW");
VerifyVersionInfoPtr RealVerifyVersionInfo = (VerifyVersionInfoPtr)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "VerifyVersionInfoW");

void DebugLog(const wchar_t* message)
{
    OutputDebugString(message);
}

void SetVersionInfo(LPOSVERSIONINFOEX lpVersionInfo)
{
    lpVersionInfo->dwMajorVersion = 5;
    lpVersionInfo->dwMinorVersion = 1;
    lpVersionInfo->dwBuildNumber = 2600;
    lpVersionInfo->dwPlatformId = 2;
}

NTSTATUS WINAPI MyRtlGetVersion(PRTL_OSVERSIONINFOW lpVersionInformation)
{
    DebugLog(L"MyRtlGetVersion called");

    NTSTATUS status = RealRtlGetVersion(lpVersionInformation);
    DebugLog(L"RealRtlGetVersion called");
    SetVersionInfo((LPOSVERSIONINFOEX)lpVersionInformation);
    lpVersionInformation->szCSDVersion[0] = L'\0';
    DebugLog(L"Version information modified");

    return status;
}

DWORD WINAPI MyGetVersion()
{
    DebugLog(L"MyGetVersion called");

    DWORD realVersion = RealGetVersion();
    DWORD spoofedVersion = (5 << 16) | (1 << 8) | LOBYTE(realVersion);

    return spoofedVersion;
}

BOOL WINAPI MyGetVersionEx(LPOSVERSIONINFO lpVersionInfo)
{
    DebugLog(L"MyGetVersionEx called");

    BOOL result = RealGetVersionEx(lpVersionInfo);
    if (result)
    {
        SetVersionInfo((LPOSVERSIONINFOEX)lpVersionInfo);
    }

    return result;
}

BOOL WINAPI MyVerifyVersionInfo(LPOSVERSIONINFOEX lpVersionInfo, DWORD dwTypeMask, DWORDLONG dwlConditionMask)
{
    DebugLog(L"MyVerifyVersionInfo called");

    SetVersionInfo(lpVersionInfo);

    BOOL result = RealVerifyVersionInfo(lpVersionInfo, dwTypeMask, dwlConditionMask);
    return result;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        DebugLog(L"DllMain Called");

        DetourRestoreAfterWith();
        DetourTransactionBegin();
        DetourUpdateThread(GetCurrentThread());
        DetourAttach(&(PVOID&)RealRtlGetVersion, MyRtlGetVersion);
        DetourAttach(&(PVOID&)RealGetVersion, MyGetVersion);
        DetourAttach(&(PVOID&)RealGetVersionEx, MyGetVersionEx);
        DetourAttach(&(PVOID&)RealVerifyVersionInfo, MyVerifyVersionInfo);
        DetourTransactionCommit();

        DebugLog(L"Hook installed successfully");
    }

    return TRUE;
}