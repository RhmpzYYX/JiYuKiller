// JiYuKillerLoader.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "JiYuKillerLoader.h"
#include "resource.h"
#include <windowsx.h>
#include <ShellAPI.h>
#include <shlwapi.h>
#include <winioctl.h>

WCHAR currentDir[MAX_PATH];
WCHAR currentFullPath[MAX_PATH];
WCHAR installDir[MAX_PATH];

WCHAR partMainExePath[MAX_PATH];
WCHAR partMainPath[MAX_PATH];
WCHAR partHtmlayoutPath[MAX_PATH];
WCHAR partVirusPath[MAX_PATH];
WCHAR partDriverPath[MAX_PATH];

HINSTANCE hInst;

bool currentIsMain = true;
bool currentShouldStartAfterInstall = false;

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,  LPWSTR lpCmdLine,  int nCmdShow)
{
	hInst = hInstance;

	Loader_GenAllPath(0);

	if (Loader_IsUsbDrv(currentDir))
	{
		WCHAR sysTempPath[MAX_PATH + 1];
		GetTempPath(MAX_PATH, sysTempPath);
		wcscpy_s(installDir, sysTempPath);
		wcscat_s(installDir, L"\\JiYuKiller\\");
		currentIsMain = false;
		currentShouldStartAfterInstall = true;
	}
	else wcscpy_s(installDir, currentDir);

	Loader_GenAllPath(1);

	if (!Loader_CheckAndInstall(installDir)) 
		return -1;

	return Loader_RunMain();
}

void Loader_GenAllPath(int o) 
{
	if (o == 0) 
	{
		GetModuleFileName(0, currentFullPath, MAX_PATH);
		GetModuleFileName(0, currentDir, MAX_PATH);
		PathRemoveFileSpec(currentDir);
	}
	else if (o == 1) 
	{
		wcscpy_s(partMainPath, installDir);
		wcscpy_s(partHtmlayoutPath, installDir);
		wcscpy_s(partVirusPath, installDir);
		wcscpy_s(partDriverPath, installDir);
		wcscpy_s(partMainExePath, installDir);
		
		wcscat_s(partMainExePath, L"\\JiYuKiller.exe");
		wcscat_s(partMainPath, L"\\JiYuKiller.dll");
		wcscat_s(partHtmlayoutPath, L"\\htmlayout.dll");
		wcscat_s(partVirusPath, L"\\JiYuKillerVirus.dll");
		wcscat_s(partDriverPath, L"\\JiYuKillerDriver.sys");

	}
}
bool Loader_IsUsbDrv(const wchar_t *path)
{
	//
	//path: "\\\\?\\F:"
#define IOCTL_STORAGE_QUERY_PROPERTY   CTL_CODE(IOCTL_STORAGE_BASE, 0x0500, METHOD_BUFFERED, FILE_ANY_ACCESS)
	typedef  struct _STORAGE_DEVICE_DESCRIPTOR
	{
		DWORD Version;                DWORD Size;
		BYTE  DeviceType;             BYTE  DeviceTypeModifier;
		BOOLEAN RemovableMedia;       BOOLEAN CommandQueueing;
		DWORD VendorIdOffset;         DWORD ProductIdOffset;
		DWORD ProductRevisionOffset;  DWORD SerialNumberOffset;
		STORAGE_BUS_TYPE BusType;     DWORD RawPropertiesLength;
		BYTE  RawDeviceProperties[1];
	} STORAGE_DEVICE_DESCRIPTOR;

	HANDLE hDisk;
	STORAGE_DEVICE_DESCRIPTOR devDesc;
	DWORD query[3] = { 0,0,1588180 };

	DWORD cbBytesReturned;

	TCHAR szBuf[300];
	wsprintf(szBuf, L"\\\\?\\%C:", path[0]);
	hDisk = CreateFile(szBuf, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hDisk == INVALID_HANDLE_VALUE)
		return false;

	if (DeviceIoControl(hDisk, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query),
		&devDesc, sizeof(devDesc), &cbBytesReturned, NULL))
	{
		if (devDesc.BusType == BusTypeUsb)
		{
			CloseHandle(hDisk);
			return true;
		}
	}
	return false;
}
bool Loader_CheckAndInstall(const wchar_t *path)
{
	if (!PathFileExists(partMainPath) && !Loader_ExtractFile(IDR_MAIN, partMainPath)) return false;
	if (!PathFileExists(partHtmlayoutPath) && !Loader_ExtractFile(IDR_HTMLAYOUT, partHtmlayoutPath)) return false;
	if (!PathFileExists(partVirusPath) && !Loader_ExtractFile(IDR_VIRUS, partVirusPath)) return false;
	if (!PathFileExists(partDriverPath) && !Loader_ExtractFile(IDR_DRIVER, partDriverPath)) return false;

	if (currentShouldStartAfterInstall) 
		if (!PathFileExists(partMainExePath) && !CopyFile(currentFullPath, partMainExePath, FALSE)) return false;

	return true;
}
bool Loader_ExtractFile(int res_id, const wchar_t *to_path)
{
	WCHAR lastError[56];

	HANDLE hFile = CreateFile(to_path, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		swprintf_s(lastError, L"安装模块 %s  时发生错误 %d", to_path, GetLastError());
		return false;
	}

	bool success = false;
	HRSRC hResource = FindResource(hInst, MAKEINTRESOURCE(res_id), TEXT("BIN"));
	if (hResource) {
		HGLOBAL hg = LoadResource(hInst, hResource);
		if (hg) {
			LPVOID pData = LockResource(hg);
			if (pData)
			{
				DWORD dwSize = SizeofResource(hInst, hResource);
				DWORD writed;
				if (WriteFile(hFile, pData, dwSize, &writed, NULL))
					success = true;
				else swprintf_s(lastError, L"写入模块 %s  时发生错误 %d", to_path, GetLastError());

				CloseHandle(hFile);
				return success;
			}
			else swprintf_s(lastError, L"提取模块资源 %s  时发生错误(LockResource) %d", to_path, GetLastError());
		}
		else swprintf_s(lastError, L"提取模块资源 %s  时发生错误(LoadResource) %d", to_path, GetLastError());
	}
	else swprintf_s(lastError, L"提取模块资源 %s  时发生错误(FindResource) %d", to_path, GetLastError());

	MessageBox(0, lastError, L"程序初始化失败", MB_ICONERROR);
	return false;
}

int Loader_RunMain() 
{
	if (currentIsMain)
	{
		HMODULE hMain = LoadLibrary(L"JiYuKiller.dll");
		if (!hMain) {
			MessageBox(0, L"没有加载主模块", L"程序初始化失败", MB_ICONERROR);
			return -1;
		}

		typedef int(*fnJRM)();

		fnJRM jrm = (fnJRM)GetProcAddress(hMain, "JRunMain");
		if (!jrm) {
			MessageBox(0, L"加载主模块已损坏", L"程序初始化失败", MB_ICONERROR);
			return -1;
		}

		return jrm();
	}
	else if (currentShouldStartAfterInstall)
		ShellExecute(0, L"runas", partMainExePath, NULL, NULL, SW_SHOW);
	return 0;
}