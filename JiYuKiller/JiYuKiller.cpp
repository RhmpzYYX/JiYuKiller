// JiYuKiller.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "JiYuKiller.h"
#include "DriverLoader.h"
#include "Executor.h"
#include "NtHlp.h"
#include "MsgCenter.h"
#include "StringHlp.h"
#include "StringSplit.h"
#include "MainWindow.h"
#include <time.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <windowsx.h>
#include <ShellAPI.h>
#include <shlwapi.h>

using namespace std;

HINSTANCE hInst;  
WCHAR currentDir[MAX_PATH];
WCHAR virusPath[MAX_PATH];
WCHAR driverPath[MAX_PATH];
WCHAR fullPath[MAX_PATH];
WCHAR iniPath[MAX_PATH];

EXTERN_C int __declspec(dllexport) JRunTest()
{
	return 0;
}
EXTERN_C int __declspec(dllexport) JRunMain() 
{
	GetModuleFileName(0, fullPath, MAX_PATH);
	GetModuleFileName(0, currentDir, MAX_PATH);
	PathRemoveFileSpec(currentDir);
	
	wcscpy_s(iniPath, fullPath);
	PathRenameExtension(iniPath, L".ini");

	wcscpy_s(virusPath, currentDir);
	wcscpy_s(driverPath, currentDir);
	wcscat_s(driverPath, L"\\JiYuKillerDriver.sys");
	wcscat_s(virusPath, L"\\JiYuKillerVirus.dll");

	LPWSTR *szArgList;
	int argCount;

	szArgList = CommandLineToArgvW(GetCommandLine(), &argCount);
	if (szArgList == NULL)
	{
		MessageBox(NULL, L"Unable to parse command line", L"Error", MB_OK);
		return -1;
	}
	if (XPreReadCommandLine(szArgList, argCount)) {
		LocalFree(szArgList);
		return 0;
	}
	LocalFree(szArgList);

	if (XInitApp())
	{
		MSG msg;
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return (int)msg.wParam;
	}

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInst = hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
