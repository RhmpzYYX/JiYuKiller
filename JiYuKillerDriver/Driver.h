#pragma once
#include <ntifs.h>
#include <Ntddkbd.h>  

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegPath);
VOID DriverUnload(_In_ struct _DRIVER_OBJECT *DriverObject);

NTSTATUS IOControlDispatch(IN PDEVICE_OBJECT pDeviceObject, IN PIRP Irp);
NTSTATUS CreateDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

VOID LoadFunctions();

NTSTATUS ZeroKill(ULONG_PTR PID);
NTSTATUS KillProcess(PEPROCESS pEThread);

VOID CompuleReBoot(void);
VOID CompuleShutdown(void);

//Defs

typedef void* (__cdecl *memset_)(void*  _Dst, int _Val, size_t _Size);
typedef int(_stdcall *memcpy_s_)(void* const _Destination, rsize_t const _DestinationSize, void const* const _Source, rsize_t const _SourceSize);
typedef int(_stdcall *strcpy_s_)(char* _Destination, rsize_t _SizeInBytes, char const* _Source);
typedef int(_stdcall *strcat_s_)(char* _Destination, rsize_t _SizeInBytes, char const* _Source);
typedef int(_stdcall *swprintf_s_)(wchar_t* _Buffer, wchar_t const* _Format, ...);
typedef int(_stdcall *wcscpy_s_)(wchar_t* _Destination, rsize_t _SizeInWords, _In_z_ wchar_t const* _Source);
typedef int(_stdcall *wcscat_s_)(wchar_t* _Destination, rsize_t _SizeInWords, _In_z_ wchar_t const* _Source);

typedef NTSTATUS(_stdcall *PspTerminateThreadByPointer_)(IN PETHREAD Thread, IN NTSTATUS ExitStatus, IN BOOLEAN DirectTerminate);
typedef NTSTATUS(_stdcall *PspExitThread_)(IN NTSTATUS ExitStatus);
typedef NTSTATUS(_stdcall *PsResumeProcess_)(PEPROCESS Process);
typedef NTSTATUS(_stdcall *PsSuspendProcess_)(PEPROCESS Process);
typedef NTSTATUS(_stdcall *PsLookupProcessByProcessId_)(HANDLE ProcessId, PEPROCESS *Process);
typedef NTSTATUS(_stdcall *PsLookupThreadByThreadId_)(HANDLE ThreadId, PETHREAD *Thread);
typedef PETHREAD(_stdcall *PsGetNextProcessThread_)(IN PEPROCESS Process, IN PETHREAD Thread);
typedef NTSTATUS(_stdcall *PsTerminateProcess_)(PEPROCESS Process, NTSTATUS ExitStatus);
typedef PEPROCESS(_stdcall *PsGetNextProcess_)(PEPROCESS Process);
typedef ULONG(_stdcall *KeForceResumeThread_)(__inout PKTHREAD Thread);
typedef NTSTATUS(_stdcall *ZwTerminateProcess_)(_In_opt_ HANDLE ProcessHandle,_In_ NTSTATUS ExitStatus);





