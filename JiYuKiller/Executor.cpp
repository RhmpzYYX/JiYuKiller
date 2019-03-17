#include  "stdafx.h"
#include  "Executor.h"
#include "JiYuKiller.h"
#include "StringHlp.h"
#include "NtHlp.h"
#include "DriverLoader.h"
#include "MsgCenter.h"
#include "../JiYuKillerDriver/IoCtl.h"
#include <ShellAPI.h>
#include <shlwapi.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <WinIoCtl.h>

extern HINSTANCE hInst;
extern HANDLE hKDrv;
extern bool noMercyMode;
extern bool forceKill;
extern bool autoVirus;
extern bool autoFck;
extern DWORD jiyuPid;
extern WCHAR jiyuPath[MAX_PATH];
extern WCHAR virusPath[MAX_PATH];
extern WCHAR driverPath[MAX_PATH];

extern HDESK hDesktop;
extern HWND hWndMain;

extern int screenWidth;
extern int screenHeight;


bool TryFindStudent(DWORD* outPid)
{
	PROCESSENTRY32 pe;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hSnapshot, &pe))
		return 0;
	while (1)
	{
		pe.dwSize = sizeof(PROCESSENTRY32);
		if (Process32Next(hSnapshot, &pe) == FALSE) break;
		if (StrEqual(pe.szExeFile, L"StudentMain.exe"))
		{
			// OpenProcess(PROCESS_ALL_ACCESS | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, pe.th32ProcessID);
			if(jiyuPid != pe.th32ProcessID) jiyuPid = pe.th32ProcessID;
			if(outPid)*outPid = pe.th32ProcessID;
			return true;
		}
	}
	jiyuPid = 0;
	CloseHandle(hSnapshot);
	return false;
}
bool TryFindXX(DWORD* outPid, const wchar_t* name)
{
	PROCESSENTRY32 pe;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hSnapshot, &pe))
		return 0;
	while (1)
	{
		pe.dwSize = sizeof(PROCESSENTRY32);
		if (Process32Next(hSnapshot, &pe) == FALSE)
			break;
		if (StrEqual(pe.szExeFile, name))
		{
			// OpenProcess(PROCESS_ALL_ACCESS | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, FALSE, pe.th32ProcessID);
			*outPid = pe.th32ProcessID;
			return true;
		}
	}
	CloseHandle(hSnapshot);
	return false;
}

void SuOthers() {
	DWORD pid = 0;
	if (TryFindXX(&pid, L"MasterHelper.exe")) {
		NTSTATUS status = 0;
		if (!KillKernelSafe(pid, &status))
			OutPutStatus(L"无法正常结束 MasterHelper.exe NTSTATUS:0x%08X", status);
	}	
	if (TryFindXX(&pid, L"GATESRV.exe")) {
		NTSTATUS status = 0;
		if (!KillKernelSafe(pid, &status))
			OutPutStatus(L"无法正常结束 MasterHelper.exe NTSTATUS:0x%08X", status);
	}
}
bool KillSt()
{
	DWORD pid ;
	if (TryFindStudent(&pid)) {
		OutPutStatus(L"已经找到 StudentMain.exe PID 为：%d 正在执行清除", pid);
		KillStReal();
	}
	else OutPutStatus(L"没有找到 StudentMain.exe");
	return false;
}
void HungSt() {
	DWORD pid;
	if (TryFindStudent(&pid)) {
		OutPutStatus(L"已经找到 StudentMain.exe PID 为：%d 正在执行挂起", pid);

		NTSTATUS status = 0;
		if (SuspendNormal(pid, &status)) {
			OutPutStatus(L"已成功暂停 Student");
			return;
		}
		else OutPutStatus(L"SuspendNormal 错误 0x%08X", status);
		if (SuspendIt(pid, &status)) {
			OutPutStatus(L"已成功强制暂停 Student");
			return;
		}
		else OutPutStatus(L"SuspendIt 错误 0x%08X", status);
	}
	else OutPutStatus(L"没有找到 StudentMain.exe");
}
void ResuseSt() {
	DWORD pid;
	if (TryFindStudent(&pid)) {
		OutPutStatus(L"已经找到 StudentMain.exe PID 为：%d 正在取消挂起", pid);

		NTSTATUS status = 0;
		if (ReusemeIt(pid, &status)) {
			OutPutStatus(L"已成功暂停 Student");
			return;
		}
		else OutPutStatus(L"ReusemeIt 错误 0x%08X", status);

	}
	else OutPutStatus(L"没有找到 StudentMain.exe");
}
bool TryFIndStPath() {
	if (jiyuPid > 4) {
		HANDLE hProcess;
		if (NT_SUCCESS(MOpenProcessNt(jiyuPid, &hProcess)))
			if (MGetProcessFullPathEx(hProcess, jiyuPath)) {
				OutPutStatus(L"已确定目标位置：%s", jiyuPath);
				if (autoVirus) {
					OutPutStatus(L"开始注入病毒");
					InstallVirus();
				}
				return true;
			}
	}
	return false;
}

bool KillStReal() {

	if (jiyuPid > 4) {
				
		if (KillNormal(jiyuPid)) {
			OutPutStatus(L"已成功杀死  StudentMain.exe");
			jiyuPid = 0;
			return true;
		}
		else {
			if (GetLastError() == ERROR_INVALID_HANDLE) {
				jiyuPid = 0;
				return true;
			}
			OutPutStatus(L"无法正常结束 StudentMain.exe，转为内核模式...");
		}

		NTSTATUS status = 0;
		if (KillKernelSafe(jiyuPid, &status)) {
			OutPutStatus(L"已成功杀死  StudentMain.exe [内核安全模式]");
			jiyuPid = 0;
			return true;
		}
		else {
			if (status == STATUS_INVALID_CID || status == STATUS_INVALID_HANDLE) {
				jiyuPid = 0;
				return true;
			}
			OutPutStatus(L"无法正常结束 StudentMain.exe NTSTATUS:0x%08X，转为内核暴力模式...", status);
		}

		if (!forceKill) {
			OutPutStatus(L"无法安全地结束 StudentMain.exe，请开启“强制结束”通知内核暴力杀死进程。");
			jiyuPid = 0;
			return false;
		}

		if (KillKernelUnsafe(jiyuPid, &status)) {
			OutPutStatus(L"已成功杀死  StudentMain.exe [内核暴力模式]");
			jiyuPid = 0;
			return true;
		}
		else {
			if (status == STATUS_INVALID_CID || status == STATUS_INVALID_HANDLE) {
				jiyuPid = 0;
				return true;
			}
			OutPutStatus(L"内核暴力无法结束 StudentMain.exe NTSTATUS:0x%08X，没辙了，看看错误是什么吧", status);
		}
	}
	return false;
}

bool SuspendNormal(DWORD pid, NTSTATUS *pStatus) {
	*pStatus = MSuspendProcessNt(pid, NULL);
	return NT_SUCCESS(*pStatus);
}
bool SuspendIt(DWORD pid, NTSTATUS *pStatus) {
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	DWORD ReturnLength = 0;
	ULONG_PTR pidb = pid;
	if (DeviceIoControl(hKDrv, CTL_SUSPEND_PROCESS, &pidb, sizeof(ULONG_PTR), &status, sizeof(status), &ReturnLength, NULL))
	{
		if (status == STATUS_SUCCESS)
			return TRUE;
	}
	if (pStatus)*pStatus = status;
	return false;
}
bool ReusemeIt(DWORD pid, NTSTATUS *pStatus) {
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	status = MResumeProcessNt(pid, NULL);
	if (!NT_SUCCESS(status) && status == STATUS_ACCESS_DENIED) {

		DWORD ReturnLength = 0;
		ULONG_PTR pidb = pid;
		if (DeviceIoControl(hKDrv, CTL_RESUME_PROCESS, &pidb, sizeof(ULONG_PTR), &status, sizeof(status), &ReturnLength, NULL))
		{
			if (status == STATUS_SUCCESS)
				return TRUE;
		}
	}
	if (pStatus)*pStatus = status;
	return NT_SUCCESS(status);
}
bool KillNormal(DWORD pid) {
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hProcess)
		return TerminateProcess(hProcess, 0);
	return false;
}
bool KillKernelSafe(DWORD pid, NTSTATUS *pStatus) {
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	DWORD ReturnLength = 0;
	ULONG_PTR pidb = pid;
	if (DeviceIoControl(hKDrv, CTL_KILL_PROCESS, &pidb, sizeof(ULONG_PTR), &status, sizeof(status), &ReturnLength, NULL))
	{
		if (status == STATUS_SUCCESS)
			return TRUE;
	}
	if (pStatus)*pStatus = status;
	return false;
}
bool KillKernelUnsafe(DWORD pid, NTSTATUS *pStatus) {
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	DWORD ReturnLength = 0;
	ULONG_PTR pidb = pid;
	if (DeviceIoControl(hKDrv, CTL_KILL_PROCESS_SPARE_NO_EFFORT, &pidb, sizeof(ULONG_PTR), &status, sizeof(status), &ReturnLength, NULL))
	{
		if (status == STATUS_SUCCESS)
			return TRUE;
	}	if (pStatus)*pStatus = status;
	return false;
}
void SuspendThread(DWORD tid) {
	HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, tid);
	if (hThread) SuspendThread(hThread);
}

void WriteHookHWNDMsg(HWND hWnd) {
	WCHAR str[65];
	if(autoFck) swprintf_s(str, L"hwf:%d", (LONG)hWnd);
	else swprintf_s(str, L"hw:%d", (LONG)hWnd);
	MsgCenterSendToVirus(str, hWndMain);
}
void FuckWindow(HWND hWnd, LPWSTR name) {
	if (hWnd == hWndMain) return;
	ForceUnTopWindow(hWnd);
	DWORD pid = 0, tid = 0;
	NTSTATUS status = 0;
	if (!noMercyMode) {
		if (!FixJIYuWindow(hWnd, name))  {
				OutPutStatus(L"执行调整 Student 窗口失败，启动强制挂起进程");

				if (SuspendNormal(pid, &status)) {
					OutPutStatus(L"已成功暂停 Student");
					return;
				}
				else OutPutStatus(L"SuspendNormal 错误 0x%08X", status);
				if (SuspendIt(pid, &status)) {
					OutPutStatus(L"已成功强制暂停 Student");
					return;
				}
				else OutPutStatus(L"SuspendIt 错误 0x%08X", status);

				if (forceKill && NT_SUCCESS(status = MTerminateThreadNt(tid, NULL))) {
					OutPutStatus(L"已成功清除 Student 窗口线程");
					return;
				}
				else OutPutStatus(L"MTerminateThreadNt 错误 0x%08X", status);
			}
	}
	else {
		OutPutStatus(L"已经找到 StudentMain.exe PID 为：%d 正在执行清除", pid);
		KillStReal();
	}
}
void ForceUnTopWindow(HWND hWnd) {
	LONG oldLong = GetWindowLong(hWnd, GWL_EXSTYLE);
	if((oldLong & WS_EX_TOPMOST)== WS_EX_TOPMOST)
		SetWindowLong(hWnd, GWL_EXSTYLE, oldLong^WS_EX_TOPMOST);
	SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}
bool FixJIYuWindow(HWND hWnd, LPWSTR name) {

	WriteHookHWNDMsg(hWnd);
	Sleep(50);
	SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) | WS_OVERLAPPEDWINDOW);
	ForceUnTopWindow(hWnd);

	if(StrEqual(name, L"BlackScreen Window")) SetWindowPos(NULL, HWND_BOTTOM, screenWidth / 2 - screenWidth / 4, screenHeight / 2 - screenHeight / 4, screenWidth / 2, screenHeight / 2, SWP_DRAWFRAME | SWP_NOACTIVATE);
	else SetWindowPos(NULL, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_DRAWFRAME | SWP_NOACTIVATE);

	return 1;
}
bool CheckIsJIYuWindow(HWND hWnd, LPDWORD outPid , LPDWORD outTid ) {
	if (jiyuPid == 0) return false;
	DWORD pid = 0, tid = GetWindowThreadProcessId(hWnd, &pid);
	if (outPid) *outPid = pid;	if (outTid) *outTid = tid;
	return pid == jiyuPid;
}
HWND TryGetJIYuFullscreenWindow() {
	HWND hWnd = GetForegroundWindow();
	if (hWnd != hWndMain && CheckIsJIYuWindow(hWnd)) return hWnd;
	return NULL;
}
void RunTopWindowCheckWk() {
	if (jiyuPid != 0) {
		HWND hWnd = TryGetJIYuFullscreenWindow();
		if (hWnd) {
			WCHAR text[32];
			GetWindowText(hWnd, text, 32);
			FuckWindow(hWnd, text);
		}
		EnumDesktopWindows(hDesktop, EnumWindowsProc, 0);
	}
}

BOOL CALLBACK EnumWindowsProc(HWND hWnd,  LPARAM lParam)
{
	if (CheckIsJIYuWindow(hWnd)) {
		WCHAR text[32];
		GetWindowText(hWnd, text, 32);
		if (StrEqual(text, L"JY Killer Virus"))
			return TRUE;
		if (autoFck) {
			RECT rc;
			GetWindowRect(hWnd, &rc);
			if (rc.top == 0 && rc.left == 0 && rc.right == screenWidth && rc.bottom == screenHeight) {
				//Full window
				FuckWindow(hWnd, text);
			}
		}
		else {
			if (StrEqual(text, L"屏幕广播") || StrEqual(text, L"BlackScreen Window")) {
				//JiYu window
				FuckWindow(hWnd, text);
			}
		}
	}
	return TRUE;
}

void FShutdownn() {
	DWORD ReturnLength = 0;
	DeviceIoControl(hKDrv, CTL_SUSPEND_PROCESS, NULL, NULL, NULL, NULL, &ReturnLength, NULL);
}

void InstallVirus() {
	if (jiyuPid > 4) {
		if (_waccess_s(virusPath, 0) == 0)
		{
			if(InjectDll(virusPath, jiyuPid))
				OutPutStatus(L"注入病毒成功");
		}
		else OutPutStatus(L"无法注入病毒，未找到文件");
	}
}
int EnableDebugPriv(const wchar_t * name)
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
	AdjustTokenPrivileges(hToken, 0, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
	return 0;
}
BOOL InjectDll(const wchar_t *DllFullPath, const DWORD dwRemoteProcessId)
{
	HANDLE hRemoteProcess;
	//打开远程线程
	hRemoteProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwRemoteProcessId);
	if (!hRemoteProcess || hRemoteProcess == INVALID_HANDLE_VALUE) {
		if (GetLastError() == ERROR_ACCESS_DENIED) {
			OutPutStatus(L"无法注入病毒，启动强制结束");	
			KillSt();
		}
	}

	wchar_t *pszLibFileRemote;

	//使用VirtualAllocEx函数在远程进程的内存地址空间分配DLL文件名空间
	pszLibFileRemote = (wchar_t *)VirtualAllocEx(hRemoteProcess, NULL, sizeof(wchar_t) * ( lstrlen(DllFullPath) + 1), MEM_COMMIT, PAGE_READWRITE);

	//使用WriteProcessMemory函数将DLL的路径名写入到远程进程的内存空间
	WriteProcessMemory(hRemoteProcess, pszLibFileRemote, (void *)DllFullPath, sizeof(wchar_t) * (lstrlen(DllFullPath) + 1), NULL);

	//##############################################################################
		//计算LoadLibraryA的入口地址
	PTHREAD_START_ROUTINE pfnStartAddr = (PTHREAD_START_ROUTINE)
		GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
	//(关于GetModuleHandle函数和GetProcAddress函数)

	//启动远程线程LoadLibraryW，通过远程线程调用创建新的线程
	HANDLE hRemoteThread;
	if ((hRemoteThread = CreateRemoteThread(hRemoteProcess, NULL, 0, pfnStartAddr, pszLibFileRemote, 0, NULL)) == NULL)
	{
		OutPutStatus(L"注入线程失败!");
		OutPutStatus(L"无法注入病毒，启动强制结束");
		KillSt();
		return FALSE;
	}
	//##############################################################################

		/*
		// 在//###.....//###里的语句也可以用如下的语句代替:
		 DWORD dwID;
		 LPVOID pFunc = LoadLibraryA;
		 HANDLE hRemoteThread = CreateRemoteThread(hRemoteProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pFunc, pszLibFileRemote, 0, &dwID );
		 //是不是感觉简单了很多
		*/

		// 释放句柄

	CloseHandle(hRemoteProcess);
	CloseHandle(hRemoteThread);

	return TRUE;
}
void SendVBoom() {
	WCHAR str[65];
	swprintf_s(str, L"ss:0");
	MsgCenterSendToVirus(str, hWndMain);
}
void SendVActive() {
	WCHAR str[65];
	swprintf_s(str, L"ssss:0");
	MsgCenterSendToVirus(str, hWndMain);
}

//Extract Parts
void ExtractParts() 
{
	if (!PathFileExists(virusPath))
	{
		if (WriteResourceToFile(IDR_VIRUS, virusPath))
			OutPutStatus(L"创建模块文件成功：%s", virusPath);
	}
	if (!PathFileExists(driverPath))
	{
		if (WriteResourceToFile(IDR_DRIVER, driverPath))
			OutPutStatus(L"创建模块文件成功：%s", driverPath);
	}
}
bool WriteResourceToFile(int rsId, LPWSTR toFile) {

	HANDLE hFile = CreateFile(toFile, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		OutPutStatus(L"创建模块文件时失败：%s  错误：%d", toFile, GetLastError());
		return false;
	}

	bool success = false;
	HRSRC hResource = FindResource(hInst, MAKEINTRESOURCE(rsId), TEXT("BIN"));
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
				else OutPutStatus(L"WriteFile 失败：%d", GetLastError());

				CloseHandle(hFile);
				return success;
			}
			else OutPutStatus(L"LockResource 失败：%d", GetLastError());
		}
		else OutPutStatus(L"LoadResource 失败：%d", GetLastError());
	}
	else OutPutStatus(L"FindResource 失败：%d", GetLastError());
	
	return false;
}

//Unload JiYuDriver
void TryForceUnloadJiYuDriver() {
	UnLoadKernelDriver(L"TDFileFilter");
	UnLoadKernelDriver(L"TDNetFilter");
}

HHOOK mouse_Hook = NULL;

//Mouse hook
BOOL InstallMouseHook()
{
	if (mouse_Hook)  UninstallMouseHook();
	mouse_Hook = SetWindowsHookEx(WH_MOUSE_LL,
		(HOOKPROC)(LowLevelMouseProc), hInst, NULL);
	return(mouse_Hook != NULL);
}
BOOL UninstallMouseHook()
{
	BOOL jud = FALSE;
	if (mouse_Hook) {
		jud = UnhookWindowsHookEx(mouse_Hook);
		mouse_Hook = NULL;  //set NULL  
	}

	return jud;
}

LRESULT CALLBACK LowLevelMouseProc(INT nCode, WPARAM wParam, LPARAM lParam)
{
	POINT  _mousepoint;
	MSLLHOOKSTRUCT *pkbhs = (MSLLHOOKSTRUCT *)lParam;
	switch (nCode)
	{
	case HC_ACTION:
	{
		//鼠标左击  
		if ((wParam == WM_MBUTTONUP || wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP)) {
			//获取鼠标的位置，并进行必要的判断

			GetCursorPos(&_mousepoint);
			HWND hWndPoint = WindowFromPoint(_mousepoint);
			LONG exstytle = GetWindowLong(hWndPoint, GWL_EXSTYLE);
			if (hWndPoint != hWndMain) {
				//Top and not this window
				DWORD pid = 0, tid = GetWindowThreadProcessId(hWndPoint, &pid);
				if (pid == GetCurrentProcessId()) break;
				//Show a dialog to ask if want to close this window

			}

			LONG exthis = GetWindowLong(hWndMain, GWL_EXSTYLE);
			if ((exthis & WS_EX_TOPMOST) != WS_EX_TOPMOST)
			{
				exthis |= WS_EX_TOPMOST;
				SetWindowLong(hWndMain, GWL_EXSTYLE, exthis);
				SetWindowPos(hWndMain, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				if (GetForegroundWindow() != hWndMain)
					SetForegroundWindow(hWndMain);
			}

		}
		break;
	}
	default:
		break;
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}