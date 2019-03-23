#include "Driver.h"
#include "IoCtl.h"
#include "UnExp.h"

#define DEVICE_LINK_NAME L"\\??\\JKRK"
#define DEVICE_OBJECT_NAME  L"\\Device\\JKRK"

ULONG gIrpCount = 0;

strcat_s_ _strcat_s;
strcpy_s_ _strcpy_s;
memcpy_s_ _memcpy_s;
swprintf_s_ swprintf_s;
wcscpy_s_ _wcscpy_s;
wcscat_s_ _wcscat_s;
memset_ _memset;
//====================================================
//
PsResumeProcess_ _PsResumeProcess;
PsSuspendProcess_ _PsSuspendProcess;
PsLookupProcessByProcessId_ _PsLookupProcessByProcessId;
PsLookupThreadByThreadId_ _PsLookupThreadByThreadId;
ZwTerminateProcess_ _ZwTerminateProcess;

extern PspTerminateThreadByPointer_ PspTerminateThreadByPointer;
extern PspExitThread_ PspExitThread;

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegPath)
{
	NTSTATUS ntStatus;

	UNICODE_STRING DeviceObjectName; // NT Device Name 
	UNICODE_STRING DeviceLinkName; // Win32 Name 
	PDEVICE_OBJECT deviceObject = NULL; 
	RtlInitUnicodeString(&DeviceObjectName, DEVICE_OBJECT_NAME);

	ntStatus = IoCreateDevice(
		pDriverObject,
		0, 
		&DeviceObjectName,
		FILE_DEVICE_UNKNOWN, 
		FILE_DEVICE_SECURE_OPEN, 
		FALSE,
		&deviceObject); 

	if (!NT_SUCCESS(ntStatus))
	{
		KdPrint(("Couldn't create the device object\n"));
		return ntStatus;
	}

	pDriverObject->DriverUnload = (PDRIVER_UNLOAD)DriverUnload;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IOControlDispatch;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = CreateDispatch;

	//创建驱动设备对象	
	RtlInitUnicodeString(&DeviceLinkName, DEVICE_LINK_NAME);
	ntStatus = IoCreateSymbolicLink(&DeviceLinkName, &DeviceObjectName);
	if (!NT_SUCCESS(ntStatus))
	{
		KdPrint(("Couldn't create symbolic link\n"));
		IoDeleteDevice(deviceObject);
	}

	LoadFunctions();

	KdPrint(("DriverEntry OK!\n"));
	return ntStatus;
}
VOID DriverUnload(_In_ struct _DRIVER_OBJECT *pDriverObject)
{
	UNICODE_STRING  DeviceLinkName;
	PDEVICE_OBJECT  v1 = NULL;
	PDEVICE_OBJECT  DeleteDeviceObject = NULL;

	RtlInitUnicodeString(&DeviceLinkName, DEVICE_LINK_NAME);
	IoDeleteSymbolicLink(&DeviceLinkName);

	DeleteDeviceObject = pDriverObject->DeviceObject;
	while (DeleteDeviceObject != NULL)
	{
		v1 = DeleteDeviceObject->NextDevice;
		IoDeleteDevice(DeleteDeviceObject);
		DeleteDeviceObject = v1;
	}

	KdPrint(("DriverUnload\n"));
}

NTSTATUS IOControlDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS Status = STATUS_SUCCESS;
	ULONG_PTR Informaiton = 0;
	PVOID InputData = NULL;
	ULONG InputDataLength = 0;
	PVOID OutputData = NULL;
	ULONG OutputDataLength = 0;
	ULONG IoControlCode = 0;
	PIO_STACK_LOCATION  IoStackLocation = IoGetCurrentIrpStackLocation(Irp);  //Irp堆栈  

	IoControlCode = IoStackLocation->Parameters.DeviceIoControl.IoControlCode;
	InputData = Irp->AssociatedIrp.SystemBuffer;
	OutputData = Irp->AssociatedIrp.SystemBuffer;
	InputDataLength = IoStackLocation->Parameters.DeviceIoControl.InputBufferLength;
	OutputDataLength = IoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

	switch (IoControlCode)
	{
	case CTL_OPEN_PROCESS: {
		ULONG_PTR pid = *(ULONG_PTR*)InputData;

		PEPROCESS pEProc;
		Status = PsLookupProcessByProcessId((HANDLE)pid, &pEProc);
		if (NT_SUCCESS(Status))
		{
			HANDLE handle;
			Status = ObOpenObjectByPointer(pEProc, 0, 0, PROCESS_ALL_ACCESS, *PsProcessType, UserMode, &handle);
			if (NT_SUCCESS(Status)) {
				_memcpy_s(OutputData, OutputDataLength, &handle, sizeof(handle));
				Status = STATUS_SUCCESS;
				Informaiton = OutputDataLength;
			}
			else KdPrint(("ObOpenObjectByPointer err : 0x%08X", Status));
			ObDereferenceObject(pEProc);
		}
		break;
	}
	case CTL_OPEN_THREAD: {
		ULONG_PTR tid = *(ULONG_PTR*)InputData;
		PETHREAD pEThread;
		Status = PsLookupThreadByThreadId((HANDLE)tid, &pEThread);
		if (NT_SUCCESS(Status))
		{
			HANDLE handle;
			Status = ObOpenObjectByPointer(pEThread, 0, 0, THREAD_ALL_ACCESS, *PsThreadType, UserMode, &handle);
			if (NT_SUCCESS(Status)) {
				_memcpy_s(OutputData, OutputDataLength, &handle, sizeof(handle));
				Informaiton = OutputDataLength;
				Status = STATUS_SUCCESS;
			}
			ObDereferenceObject(pEThread);
		}
		break;
	}
	case CTL_KILL_PROCESS: {
		ULONG_PTR pid = *(ULONG_PTR*)InputData;
		PEPROCESS pEProcess;
		KdPrint(("CTL_KILL_PROCESS : %d\n", pid));
		Status = PsLookupProcessByProcessId((HANDLE)pid, &pEProcess);
		if (NT_SUCCESS(Status))
		{
			HANDLE handle;
			Status = ObOpenObjectByPointer(pEProcess, 0, 0, PROCESS_ALL_ACCESS, *PsProcessType, UserMode, &handle);
			if (NT_SUCCESS(Status))
				Status = ZwTerminateProcess(handle, STATUS_SUCCESS);
			ZwClose(handle);
			if(Status == STATUS_ACCESS_DENIED)
				Status = KillProcess(pEProcess);
			ObDereferenceObject(pEProcess);

			Status = STATUS_SUCCESS;
			_memcpy_s(OutputData, OutputDataLength, &Status, sizeof(Status));
			Informaiton = OutputDataLength;
		}
		break;
	}
	case CTL_KILL_PROCESS_SPARE_NO_EFFORT: {
		ULONG_PTR pid = *(ULONG_PTR*)InputData;
		Status = ZeroKill(pid);
		break;
	}
	case CTL_KILL_THREAD: {
		ULONG_PTR tid = *(ULONG_PTR*)InputData;
		if (PspTerminateThreadByPointer) {
			PETHREAD pEThread;
			Status = PsLookupThreadByThreadId((HANDLE)tid, &pEThread);
			if (NT_SUCCESS(Status))
			{
				Status = PspTerminateThreadByPointer(pEThread, 0, TRUE);
				ObDereferenceObject(pEThread);
			}
		}
		else Status = STATUS_NOT_SUPPORTED;
		break;
	}
	case CTL_SUSPEND_PROCESS: {
		ULONG_PTR pid = *(ULONG_PTR*)InputData;
		PEPROCESS pEProc;
		Status = PsLookupProcessByProcessId((HANDLE)pid, &pEProc);
		if (NT_SUCCESS(Status))
		{
			Status = _PsSuspendProcess(pEProc);
			ObDereferenceObject(pEProc);
		}
		_memcpy_s(OutputData, OutputDataLength, &Status, sizeof(Status));
		Informaiton = OutputDataLength;
		break;
	}
	case CTL_RESUME_PROCESS: {
		ULONG_PTR pid = *(ULONG_PTR*)InputData;
		PEPROCESS pEProc;
		Status = PsLookupProcessByProcessId((HANDLE)pid, &pEProc);
		if (NT_SUCCESS(Status))
		{
			Status = _PsResumeProcess(pEProc);
			ObDereferenceObject(pEProc);
		}
		_memcpy_s(OutputData, OutputDataLength, &Status, sizeof(Status));
		Informaiton = OutputDataLength;
		break;
	}
	case CTL_SHUTDOWN: {
		KdPrint(("CompuleShutdown\n"));
		CompuleShutdown();
		//
		break;
	}
	default: break;
	}

	Irp->IoStatus.Status = Status; //Ring3 GetLastError();
	Irp->IoStatus.Information = Informaiton;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);  //将Irp返回给Io管理器
	return Status; //Ring3 DeviceIoControl()返回值
}
NTSTATUS CreateDispatch(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp)
{
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

VOID LoadFunctions() 
{
	UNICODE_STRING MemsetName;
	UNICODE_STRING MemcpysName;
	UNICODE_STRING StrcpysName;
	UNICODE_STRING StrcatsName;
	UNICODE_STRING SWprintfsName;
	UNICODE_STRING WCscatsName;
	UNICODE_STRING WCscpysName;
	UNICODE_STRING PsResumeProcessName;
	UNICODE_STRING PsSuspendProcessName;
	UNICODE_STRING PsLookupProcessByProcessIdName;
	UNICODE_STRING PsLookupThreadByThreadIdName;
	UNICODE_STRING ZwTerminateProcessName;

	RtlInitUnicodeString(&ZwTerminateProcessName, L"ZwTerminateProces");
	RtlInitUnicodeString(&PsLookupProcessByProcessIdName, L"PsLookupProcessByProcessId");
	RtlInitUnicodeString(&PsLookupThreadByThreadIdName, L"PsLookupThreadByThreadId");
	RtlInitUnicodeString(&PsResumeProcessName, L"PsResumeProcess");
	RtlInitUnicodeString(&PsSuspendProcessName, L"PsSuspendProcess");
	RtlInitUnicodeString(&MemsetName, L"memset");
	RtlInitUnicodeString(&WCscatsName, L"wcscat_s");
	RtlInitUnicodeString(&WCscpysName, L"wcscpy_s");
	RtlInitUnicodeString(&SWprintfsName, L"swprintf_s");
	RtlInitUnicodeString(&MemcpysName, L"memcpy_s");
	RtlInitUnicodeString(&StrcpysName, L"strcpy_s");
	RtlInitUnicodeString(&StrcatsName, L"strcat_s");

	_PsLookupProcessByProcessId = (PsLookupProcessByProcessId_)MmGetSystemRoutineAddress(&PsLookupProcessByProcessIdName);
	_PsLookupThreadByThreadId = (PsLookupThreadByThreadId_)MmGetSystemRoutineAddress(&PsLookupThreadByThreadIdName);
	_PsResumeProcess = (PsResumeProcess_)MmGetSystemRoutineAddress(&PsResumeProcessName);
	_PsSuspendProcess = (PsSuspendProcess_)MmGetSystemRoutineAddress(&PsSuspendProcessName);
	_memset = (memset_)MmGetSystemRoutineAddress(&MemsetName);
	_wcscpy_s = (wcscpy_s_)MmGetSystemRoutineAddress(&WCscpysName);
	_wcscat_s = (wcscat_s_)MmGetSystemRoutineAddress(&WCscatsName);
	_memcpy_s = (memcpy_s_)MmGetSystemRoutineAddress(&MemcpysName);
	_strcat_s = (strcat_s_)MmGetSystemRoutineAddress(&StrcatsName);
	_strcpy_s = (strcpy_s_)MmGetSystemRoutineAddress(&StrcpysName);
	swprintf_s = (swprintf_s_)MmGetSystemRoutineAddress(&SWprintfsName);
	_ZwTerminateProcess = (ZwTerminateProcess_)MmGetSystemRoutineAddress(&ZwTerminateProcessName);

	PspTerminateThreadByPointer = (PspTerminateThreadByPointer_)KxGetPspTerminateThreadByPointerAddressX_7Or8Or10(0x50);
	PspExitThread = (PspExitThread_)KxGetPspExitThread_32_64();

	DbgPrint("PspTerminateThreadByPointer : 0x%08x", PspTerminateThreadByPointer);
	DbgPrint("PspExitThread : 0x%08x", PspExitThread);
}

NTSTATUS ZeroKill(ULONG_PTR PID)   //X32  X64
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	int i = 0;
	PVOID handle;
	PEPROCESS Eprocess;
	ntStatus = PsLookupProcessByProcessId((HANDLE)PID, &Eprocess);
	if (NT_SUCCESS(ntStatus))
	{
		PKAPC_STATE pKs = (PKAPC_STATE)ExAllocatePool(NonPagedPool, sizeof(PKAPC_STATE));
		KeStackAttachProcess(Eprocess, pKs);//Attach进程虚拟空间
		for (i = 0; i <= 0x7fffffff; i += 0x1000)
		{
			//__try {
				if (MmIsAddressValid((PVOID)i))
				{
					ProbeForWrite((PVOID)i, 0x1000, sizeof(ULONG));
					memset((PVOID)i, 0xcc, 0x1000);
				}
				else {
					if (i > 0x1000000)  //填这么多足够破坏进程数据了  
						break;
				}
			/*}
			__except (1) {
				KdPrint(("---错误!---"));
			}*/
		}
		KeUnstackDetachProcess(pKs); 
		ntStatus = ObOpenObjectByPointer((PVOID)Eprocess, 0, NULL, 0, NULL, KernelMode, &handle);
		if (ntStatus != STATUS_SUCCESS)
			return ntStatus;
		ZwTerminateProcess((HANDLE)handle, STATUS_SUCCESS);
		ZwClose((HANDLE)handle);
		return STATUS_SUCCESS;
	}
	return ntStatus;
}
NTSTATUS KillProcess(PEPROCESS pEProcess)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	if (!PspTerminateThreadByPointer) return STATUS_NOT_SUPPORTED;

	_PsSuspendProcess(pEProcess);
	for (UINT32 i = 8; i < 65536; i += 4)
	{
		//__try {
			PETHREAD pEThread;
			status = PsLookupThreadByThreadId((HANDLE)i, &pEThread);
			if (NT_SUCCESS(status))
			{
				if (IoThreadToProcess(pEThread) == pEProcess)
					status = PspTerminateThreadByPointer(pEThread, 0, TRUE);
				ObDereferenceObject(pEThread);
			}
		/*}
		__except (1) {
			KdPrint(("---错误!---"));
		}*/
	}
	_PsResumeProcess(pEProcess);
	status = STATUS_SUCCESS;
	return status;
}

//关闭计算机(强制)
VOID CompuleShutdown(void)
{
	typedef void(__fastcall*FCRB)(void);

	/*
	mov ax,2001h
	mov dx,1004h
	out dx,ax
	retn
	*/

	FCRB fcrb = NULL;
	UCHAR *shellcode = "\x66\xB8\x01\x20\x66\xBA\x04\x10\x66\xEF\xC3";
	fcrb = (FCRB)ExAllocatePool(NonPagedPool, 13);
	memcpy(fcrb, shellcode, 13);
	fcrb();
}