/*
 * Copyright (c) 2021 Soar Qin<soarchin@gmail.com>
 *
 * Use of this source code is governed by an MIT-style
 * license that can be found in the LICENSE file or at
 * https://opensource.org/licenses/MIT.
 */

#pragma once

#include <windows.h>

#if defined(_M_IX86)

#define NT_SUCCESS(x) ((x) >= 0)

// Namespace is present Not to collide with "winbase.h"
// definition of PROCESS_INFORMATION_CLASS and others.
namespace util {

typedef enum _PROCESS_INFORMATION_CLASS {
    ProcessBasicInformation,
    ProcessQuotaLimits,
    ProcessIoCounters,
    ProcessVmCounters,
    ProcessTimes,
    ProcessBasePriority,
    ProcessRaisePriority,
    ProcessDebugPort,
    ProcessExceptionPort,
    ProcessAccessToken,
    ProcessLdtInformation,
    ProcessLdtSize,
    ProcessDefaultHardErrorMode,
    ProcessIoPortHandlers,
    ProcessPooledUsageAndLimits,
    ProcessWorkingSetWatch,
    ProcessUserModeIOPL,
    ProcessEnableAlignmentFaultFixup,
    ProcessPriorityClass,
    ProcessWx86Information,
    ProcessHandleCount,
    ProcessAffinityMask,
    ProcessPriorityBoost,
    MaxProcessInfoClass
} PROCESS_INFORMATION_CLASS, *PPROCESS_INFORMATION_CLASS;

// ------------------------------------------------------------------------
// Structs.
// ------------------------------------------------------------------------

typedef struct _PROCESS_BASIC_INFORMATION64 {
    ULONGLONG Reserved1;
    ULONGLONG PebBaseAddress;
    ULONGLONG Reserved2[2];
    ULONGLONG UniqueProcessId;
    ULONGLONG Reserved3;
} PROCESS_BASIC_INFORMATION64;

typedef struct _PEB_LDR_DATA64 {
    ULONG Length;
    BOOLEAN Initialized;
    ULONGLONG SsHandle;
    LIST_ENTRY64 InLoadOrderModuleList;
    LIST_ENTRY64 InMemoryOrderModuleList;
    LIST_ENTRY64 InInitializationOrderModuleList;
} PEB_LDR_DATA64, *PPEB_LDR_DATA64;

// Structure is cut down to ProcessHeap.
typedef struct _PEB64 {
    BOOLEAN InheritedAddressSpace;
    BOOLEAN ReadImageFileExecOptions;
    BOOLEAN BeingDebugged;
    BOOLEAN Spare;
    ULONGLONG Mutant;
    ULONGLONG ImageBaseAddress;
    ULONGLONG LoaderData;
    ULONGLONG ProcessParameters;
    ULONGLONG SubSystemData;
    ULONGLONG ProcessHeap;
} PEB64;

typedef struct _UNICODE_STRING64 {
    USHORT Length;
    USHORT MaximumLength;
    ULONGLONG Buffer;
} UNICODE_STRING64;

typedef struct _LDR_DATA_TABLE_ENTRY64 {
    LIST_ENTRY64 InLoadOrderModuleList;
    LIST_ENTRY64 InMemoryOrderModuleList;
    LIST_ENTRY64 InInitializationOrderModuleList;
    ULONGLONG BaseAddress;
    ULONGLONG EntryPoint;
    DWORD64 SizeOfImage;
    UNICODE_STRING64 FullDllName;
    UNICODE_STRING64 BaseDllName;
    ULONG Flags;
    SHORT LoadCount;
    SHORT TlsIndex;
    LIST_ENTRY64 HashTableEntry;
    ULONGLONG TimeDateStamp;
} LDR_DATA_TABLE_ENTRY64, *PLDR_DATA_TABLE_ENTRY64;

}

#endif

// ------------------------------------------------------------------------
// Function prototypes.
// ------------------------------------------------------------------------
extern "C" {

#if defined(_M_IX86)

typedef NTSTATUS (NTAPI *NtWow64QueryInformationProcess64Proc)(
    IN HANDLE ProcessHandle,
    ULONG ProcessInformationClass,
    OUT PVOID ProcessInformation,
    IN ULONG ProcessInformationLength,
    OUT PULONG ReturnLength OPTIONAL);

typedef NTSTATUS (NTAPI *NtWow64ReadVirtualMemory64Proc)(
    IN HANDLE ProcessHandle,
    IN DWORD64 BaseAddress,
    OUT PVOID Buffer,
    IN ULONG64 Size,
    OUT PDWORD64 NumberOfBytesRead);

extern NtWow64QueryInformationProcess64Proc NtWow64QueryInformationProcess64;
extern NtWow64ReadVirtualMemory64Proc NtWow64ReadVirtualMemory64;

#endif

extern void osInit();

}
