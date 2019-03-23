#include "stdafx.h"
#include "Utils.h"

BOOL _Is64BitOS = -1;
BOOL _IsRunasAdmin = -1;

BOOL EnableDebugPriv(const wchar_t * name)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	LUID luid;
	//打开进程令牌环
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	//获得进程本地唯一ID
	LookupPrivilegeValue(NULL, name, &luid);

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	tp.Privileges[0].Luid = luid;
	//调整权限
	return AdjustTokenPrivileges(hToken, 0, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
}
BOOL MIsRunasAdmin()
{
	if (_IsRunasAdmin = -1)
	{
		BOOL bElevated = FALSE;
		HANDLE hToken = NULL;

		// Get current process token
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
			return FALSE;

		TOKEN_ELEVATION tokenEle;
		DWORD dwRetLen = 0;

		// Retrieve token elevation information
		if (GetTokenInformation(hToken, TokenElevation, &tokenEle, sizeof(tokenEle), &dwRetLen))
		{
			if (dwRetLen == sizeof(tokenEle))
			{
				bElevated = tokenEle.TokenIsElevated;
			}
		}

		CloseHandle(hToken);
		_IsRunasAdmin = bElevated;
	}
	return _IsRunasAdmin;
}
BOOL MIs64BitOS()
{
	if (_Is64BitOS == -1)
	{
		BOOL bRetVal = FALSE;
		SYSTEM_INFO si = { 0 };

		GetNativeSystemInfo(&si);
		if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
			si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 ||
			si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64)
			bRetVal = TRUE;
		_Is64BitOS = bRetVal;
	}
	return _Is64BitOS;
}
