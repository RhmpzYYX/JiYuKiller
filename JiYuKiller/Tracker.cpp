#include "stdafx.h"
#include "Tracker.h"
#include "WindowResolver.h"
#include "MainWindow.h"
#include "MsgCenter.h"
#include "NtHlp.h"
#include "DriverLoader.h"
#include "KernelUtils.h"
#include <Psapi.h>
#include <tlhelp32.h>
#include <shellapi.h>
#include <shlwapi.h>
#include "StringHlp.h"

DWORD jiyuPid = 0;
WCHAR jiyuPath[MAX_PATH] = { 0 };

extern HWND hWndMain;
extern WCHAR virusPath[MAX_PATH];

WCHAR failStatText[36] = { 0 };
WCHAR ctlStatText[512] = { 0 };
WCHAR procStatText[36] = { 0 };

bool setAutoForceKill = false;

int msgCenterRetryCount = 0;

LPWSTR TGetLastError() { return failStatText; }
DWORD TGetLastJiYuPid() { return jiyuPid; }
LPWSTR TGetLastJiYuPayh() { return jiyuPath; }
bool TFindTarget(LPDWORD outPid, bool *isNewState)
{
	PROCESSENTRY32 pe;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hSnapshot, &pe)) return 0;
	while (1)
	{
		pe.dwSize = sizeof(PROCESSENTRY32);
		if (Process32Next(hSnapshot, &pe) == FALSE) break;
		if (StrEqual(pe.szExeFile, L"StudentMain.exe"))
		{
			// OpenProcess(PROCESS_ALL_ACCESS | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, pe.th32ProcessID);
			if (jiyuPid != pe.th32ProcessID) {
				jiyuPid = pe.th32ProcessID;
				if (isNewState) *isNewState = true;
			}
			if (outPid)*outPid = pe.th32ProcessID;
			return true;
		}
	}
	if (jiyuPid != 0)
	{
		if (isNewState) *isNewState = true;
		jiyuPid = 0;
	}
	CloseHandle(hSnapshot);
	return false;
}

bool TLocated() {
	return jiyuPid != 0;
}
LPWSTR TGetCurrStatText() {
	return ctlStatText;
}
LPWSTR TGetCurrProcStatText() {
	return procStatText;
}
bool TReset()
{
	msgCenterRetryCount = 0;
	bool newState = false;
	TFindTarget(NULL, &newState);
	if (newState)
	{
		if (jiyuPid != 0) 
		{
			swprintf_s(procStatText, L"已锁定极域主进程 %d (0x%04x)", jiyuPid, jiyuPid);
			XOutPutStatus(procStatText);
			if(TFindJiYuInstallLocation(true))
				XOutPutStatus(L"已锁定极域可执行文件路径：%s", jiyuPath);
			TInstallVirus();
		}
		else {
			wcscpy_s(procStatText, L"未找到极域进程");
			XOutPutStatus(procStatText);
		}
	}
	return newState;
}
int TRunCK()
{
	return WRunCk();
}
void TSendCtlStat() 
{
	MsgCenterSendToVirus((LPWSTR)L"hk:ckstat", hWndMain);
}
void TSendCkEnd()
{
	MsgCenterSendToVirus((LPWSTR)L"hk:ckend", hWndMain);
}
void TSendBoom()
{
	MsgCenterSendToVirus((LPWSTR)L"ss:0", hWndMain);
}

void TFindJiYuPath()
{
	if (wcscmp(jiyuPath, L"") == 0) {
		if (TFindJiYuInstallLocation(false))
			XOutPutStatus(L"已锁定极域可执行文件路径：%s", jiyuPath);
	}
}
bool TFindJiYuInstallLocation(bool usePid)
{
	if (usePid && jiyuPid > 4) {
		HANDLE hProcess;
		if (NT_SUCCESS(MOpenProcessNt(jiyuPid, &hProcess)))
			if (MGetProcessFullPathEx(hProcess, jiyuPath))
				return true;
	}
	else {
		const wchar_t* path = L"C:\\Program Files\\Mythware\\e-Learning Class\\StudentMain.exe";
		if (PathFileExists(path)) {
			wcscpy_s(jiyuPath, path);
			return TRUE;
		}
		path = L"C:\\Program Files\\Mythware\\极域课堂管理系统软件V6.0 2016 豪华版\\StudentMain.exe";
		if (PathFileExists(path)) {
			wcscpy_s(jiyuPath, path);
			return TRUE;
		}
	}
	return false;
}

bool TInstallVirus()
{
	if (jiyuPid != 0)
	{
		if (TInstallVirusWithRemoteThread()) {
			XOutPutStatus(L"向极域注入远程线程成功");
			return true;
		}
		if (TInstallVirusWithDllApi()) {
			XOutPutStatus(L"向极域替换病毒成功");
			return true;
		}
	}
	return false;
}
bool TInstallVirusWithRemoteThread()
{
	HANDLE hRemoteProcess;
	//打开进程
	NTSTATUS ntStatus = MOpenProcessNt(jiyuPid, &hRemoteProcess);
	if (!NT_SUCCESS(ntStatus))
	{
		if (ntStatus == STATUS_ACCESS_DENIED)
		{
			XOutPutStatus(L"无法注入病毒 打开进程错误 : 0x%08X", jiyuPid);
			if (setAutoForceKill || MessageBox(hWndMain, L"无法注入病毒 打开进程错误。\n是否启动强制结束？", L"错误", MB_YESNO | MB_ICONEXCLAMATION)) {
				XOutPutStatus(L"无法注入病毒 启动强制结束");
				TForceKill();
			}
		}
	}

	wchar_t *pszLibFileRemote;

	//使用VirtualAllocEx函数在远程进程的内存地址空间分配DLL文件名空间
	pszLibFileRemote = (wchar_t *)VirtualAllocEx(hRemoteProcess, NULL, sizeof(wchar_t) * (lstrlen(virusPath) + 1), MEM_COMMIT, PAGE_READWRITE);

	//使用WriteProcessMemory函数将DLL的路径名写入到远程进程的内存空间
	WriteProcessMemory(hRemoteProcess, pszLibFileRemote, (void *)virusPath, sizeof(wchar_t) * (lstrlen(virusPath) + 1), NULL);

	//##############################################################################
		//计算LoadLibraryA的入口地址
	PTHREAD_START_ROUTINE pfnStartAddr = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
	//(关于GetModuleHandle函数和GetProcAddress函数)

	//启动远程线程LoadLibraryW，通过远程线程调用创建新的线程
	HANDLE hRemoteThread;
	if ((hRemoteThread = CreateRemoteThread(hRemoteProcess, NULL, 0, pfnStartAddr, pszLibFileRemote, 0, NULL)) == NULL)
	{
		XOutPutStatus(L"注入线程失败!");
		if (setAutoForceKill || MessageBox(hWndMain, L"无法注入病毒 注入线程失败。\n是否启动强制结束？", L"错误", MB_YESNO | MB_ICONEXCLAMATION)) {
			XOutPutStatus(L"无法注入病毒 启动强制结束");
			TForceKill();
		}
		return FALSE;
	}


	// 释放句柄

	CloseHandle(hRemoteProcess);
	CloseHandle(hRemoteThread);

	return TRUE;
}
bool TInstallVirusWithDllApi()
{
	if (!PathFileExists(virusPath))
	{
		XOutPutStatus(L"病毒源文件未找到，法执行替换");
		MessageBox(hWndMain, L"病毒源文件未找到，无法执行病毒替换", L"错误", MB_ICONEXCLAMATION);
		return false;
	}

	//Get base dir
	if (StrEmepty(jiyuPath))
	{
	RECHOOSE:
		if (!M_DLG_ChooseFileSingal(hWndMain, L"", L"选择 StudentMain.exe 的位置：", L"所有文件\0*.*\0Exe Flie\0*.exe\0\0", L"StudentMain.exe", L".exe", jiyuPath, MAX_PATH))
			return false;
		if (_waccess_s(jiyuPath, 0) != 0) {
			if (MessageBox(hWndMain, L"您选择的路径不存在或无法访问，点击重试重新选择", L"错误", MB_RETRYCANCEL) == IDRETRY)
				goto RECHOOSE;
		}
	}

	//Replace stub
	WCHAR jiyuDirPath[MAX_PATH];
	wcscpy_s(jiyuDirPath, jiyuPath);
	PathRemoveFileSpec(jiyuDirPath);

	WCHAR jiyuTDAPath[MAX_PATH];
	WCHAR jiyuTDAOPath[MAX_PATH];
	wcscpy_s(jiyuTDAPath, jiyuDirPath);
	wcscat_s(jiyuTDAPath, L"\\LibTDAjust.dll");
	wcscpy_s(jiyuTDAOPath, jiyuDirPath);
	wcscat_s(jiyuTDAOPath, L"\\LibTDAjust.dll.bak.dll");

	if (!PathFileExists(jiyuTDAPath)) {
		XOutPutStatus(L"目标文件未找到：%s，无法执行替换", jiyuTDAPath);
		MessageBox(hWndMain, L"目标文件未找到，无法执行病毒替换", L"错误", MB_ICONEXCLAMATION);
		return false;
	}

	//Copy backup file
	if (!CopyFile(jiyuTDAPath, jiyuTDAOPath, FALSE)) {
		XOutPutStatus(L"复制备份文件失败：%s，无法执行替换", jiyuTDAOPath);
		MessageBox(hWndMain, L"复制备份文件失败，无法执行病毒替换", L"错误", MB_ICONEXCLAMATION);
		return false;
	}

	//Delete old
	if (!DeleteFile(jiyuTDAPath))
	{
		XOutPutStatus(L"删除源目标失败：%s，无法执行替换", jiyuTDAPath);
		MessageBox(hWndMain, L"删除源目标失败，无法执行病毒替换", L"错误", MB_ICONEXCLAMATION);
		return false;
	}

	//Copy to new
	if (!CopyFile(virusPath, jiyuTDAPath, FALSE)) {
		XOutPutStatus(L"复制病毒到目标文件失败：%s，无法执行替换", jiyuTDAOPath);
		MessageBox(hWndMain, L"复制病毒到目标文件失败，无法执行病毒替换", L"错误", MB_ICONEXCLAMATION);
		return false;
	}
	return true;
}

bool TForceKill() 
{
	if (jiyuPid == 0) {
		wcscpy_s(failStatText, L"未找到极域主进程");
		XOutPutStatus(failStatText);
		return false;
	}
	HANDLE hProcess;
	NTSTATUS status = MOpenProcessNt(jiyuPid, &hProcess);
	if (!NT_SUCCESS(status)) {
		swprintf_s(failStatText, L"打开进程错误：0x%08X，请手动结束", status);
		XOutPutStatus(failStatText);
		return FALSE;
	}
	status = MTerminateProcessNt(0, hProcess);
	if (NT_SUCCESS(status)) {
		CloseHandle(hProcess);
		return TRUE;
	}
	else {
		if (status == STATUS_ACCESS_DENIED) 
			goto FORCEKILL;
		else if(status != STATUS_INVALID_CID && status != STATUS_INVALID_HANDLE) {
			swprintf_s(failStatText, L"结束进程错误：0x%08X，请手动结束", status);
			XOutPutStatus(failStatText);
			return false;
		}
	}

	FORCEKILL:
	if (DriverLoaded() && MessageBox(hWndMain, L"普通无法结束极域，是否调用驱动结束极域？", L"提示", MB_ICONEXCLAMATION | MB_YESNO) == IDYES)
	{
		if (KForceKill(jiyuPid, &status))
			return true;
		else MessageBox(hWndMain, L"驱动也无法结束，请使用 PCHunter 结束它吧！", L"错误", MB_ICONEXCLAMATION);
	}

	FAIL:
	CloseHandle(hProcess);
	return false;
}