#include "pch.h"
#include <Windows.h>
#include <winternl.h>

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#endif

extern "C" NTSTATUS NTAPI RtlGetVersion(PRTL_OSVERSIONINFOW lpVersionInformation)
{
    if (!lpVersionInformation || lpVersionInformation->dwOSVersionInfoSize < sizeof(RTL_OSVERSIONINFOW))
    {
        return STATUS_INVALID_PARAMETER;
    }

    lpVersionInformation->dwMajorVersion = 5; // Set desired major version
    lpVersionInformation->dwMinorVersion = 1; // Set desired minor version
    lpVersionInformation->dwBuildNumber = 2600; // Set desired build number
    lpVersionInformation->dwPlatformId = 2; // Set desired platform ID
    lpVersionInformation->szCSDVersion[0] = L'\0'; // Set desired service pack

    return STATUS_SUCCESS;
}
