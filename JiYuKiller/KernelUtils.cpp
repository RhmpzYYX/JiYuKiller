#include "stdafx.h"
#include "DriverLoader.h"
#include "KernelUtils.h"
#include "NtHlp.h"
#include "JiYuKiller.h"
#include <winioctl.h>
#include "../JiYuKillerDriver/IoCtl.h"

extern HANDLE hKDrv;

bool KFShutdown()
{
	if (DriverLoaded())
	{
		DWORD ReturnLength = 0;
		return DeviceIoControl(hKDrv, CTL_SHUTDOWN, NULL, 0, NULL, 0, &ReturnLength, NULL);
	}
	else XOutPutStatus(L"Çý¶¯Î´¼ÓÔØ£¡");
	return false;
}
bool KForceKill(DWORD pid, NTSTATUS *pStatus)
{
	if (DriverLoaded())
	{
		NTSTATUS status = STATUS_UNSUCCESSFUL;
		DWORD ReturnLength = 0;
		ULONG_PTR pidb = pid;
		if (DeviceIoControl(hKDrv, CTL_KILL_PROCESS, &pidb, sizeof(ULONG_PTR), &status, sizeof(status), &ReturnLength, NULL))
		{
			if (status == STATUS_SUCCESS)
				return TRUE;
			else XOutPutStatus(L"CTL_KILL_PROCESS ´íÎó£º0x08%X", status);
		}
		else XOutPutStatus(L"DeviceIoControl CTL_KILL_PROCESS ´íÎó£º%d", GetLastError());
		if (pStatus)*pStatus = status;
	}
	else XOutPutStatus(L"Çý¶¯Î´¼ÓÔØ£¡");
	return false;
}

