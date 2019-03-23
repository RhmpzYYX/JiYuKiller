#include "..\JiYuKiller\JiYuKiller.h"
// JiYuKillerVirus.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "JiYuKillerVirus.h"
#include "resource.h"
#include "StringHlp.h"
#include "StringSplit.h"
#include "mhook-lib/mhook.h"
#include <list>
#include <string>
#include <time.h>
#include <CommCtrl.h>
#include <windowsx.h>

#define TIMER_WATCH_DOG_SRV 10011
#define TIMER_AUTO_HIDE 10012

using namespace std;

extern HINSTANCE hInst;

WNDPROC jiYuWndProc;
WNDPROC jiYuTDDeskWndProc;
list<HWND>  jiYuWnds;
list<HWND>  jiYuWndCanSize;
HWND jiYuGBWnd = NULL;
HWND jiYuGBDeskRdWnd = NULL;

HWND hWndMsgCenter = NULL;
HWND hListBoxStatus = NULL;

INT screenWidth, screenHeight;

fnTDAjustCreateInstance faTDAjustCreateInstance = NULL;

bool loaded = false;

void VUnloadAll() {
	if (loaded)
	{
		VCloseMsgCenter();
		VCloseFuckDrivers();
		jiYuWnds.clear();
		jiYuWndCanSize.clear();
		VUnInstallHooks();
		loaded = false;
	}
}
void VLoad() {
	VParamInit();

	//Get main mod name
	WCHAR mainModName[MAX_PATH];
	GetModuleFileName(NULL, mainModName, MAX_PATH);

	std::wstring path(mainModName);

	int lastQ = path.find_last_of(L'\\');
	std::wstring name = path.substr(lastQ + 1, path.length() - lastQ - 1);

	if (name == L"StudentMain.exe") {
		//This is target, run virus

		GetModuleFileName(hInst, mainModName, MAX_PATH);
		path = std::wstring(mainModName);
		lastQ = path.find_last_of(L'\\');
		name = path.substr(lastQ + 1, path.length() - lastQ - 1);

		if (name == L"LibTDAjust.dll") { //Current is virus stub dll , load real and alloc
			VLoadRealVirus();
			VRunMain();
		}
		else if (name == L"JiYuKillerVirus.dll") {//Current is virus main dll
			VRunMain();
		}
	}

	loaded = true;
}
void VRunMain() {
	
	VCreateMsgCenter();
	VOpenFuckDrivers();
	VInstallHooks();
	VSendMessageBack(L"hkb:succ", hWndMsgCenter);
}
void VLoadRealVirus() {
	if (_waccess_s(L"LibTDAjust.dll.bak.dll", 0) == 0)
	{
		HMODULE hrealTDAjust = LoadLibrary(L"LibTDAjust.dll.bak.dll");
		if (!hrealTDAjust) {
			MessageBox(0, L"!hrealTDAjust ", L"ERROR!", MB_ICONERROR);
			ExitProcess(0);
		}
		faTDAjustCreateInstance = (fnTDAjustCreateInstance)GetProcAddress(hrealTDAjust, "TDAjustCreateInstance");
		if (!faTDAjustCreateInstance) {
			MessageBox(0, L"!faTDAjustCreateInstance", L"ERROR!", MB_ICONERROR);
			ExitProcess(0);
		}
	}
	else {
		MessageBox(0, L"!LibTDAjust.dll.bak.dll", L"ERROR!", MB_ICONERROR);
		ExitProcess(0);
		
	}
}

DWORD WINAPI VMsgCenterRunThread(LPVOID lpThreadParameter) {
	hWndMsgCenter = CreateDialog(hInst, MAKEINTRESOURCE(IDD_MSGCT), NULL, MainWndProc);
	ShowWindow(hWndMsgCenter, SW_SHOW);
	UpdateWindow(hWndMsgCenter);

	hListBoxStatus = GetDlgItem(hWndMsgCenter, IDC_STATUS_LIST);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

void VParamInit() {
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);
}
void VCreateMsgCenter() {
	CreateThread(NULL, NULL, VMsgCenterRunThread, NULL, NULL, NULL);
}
void VCloseMsgCenter() {
	DestroyWindow(hWndMsgCenter);
}
void VHandleMsg(LPWSTR buff) {
	VOutPutStatus(L"Receive message : %s", buff);
	wstring act(buff);
	vector<wstring> arr;
	SplitString(act, arr, L":");
	if (arr.size() >= 2) {
		if (arr[0] == L"hw") 
			VHookWindow(arr[1].c_str());
		else if (arr[0] == L"hwf")
			VHookFWindow(arr[1].c_str());
		else if (arr[0] == L"ss")
			VBoom();
		else if (arr[0] == L"hk") {
			if(arr[1]==L"ckstat") VSendMessageBack(L"hkb:succ", hWndMsgCenter);
			else if (arr[1] == L"ckend") VManualQuit();
		}
	}
}
void VOutPutStatus(const wchar_t* str, ...) {
	time_t time_log = time(NULL);
	struct tm tm_log;
	localtime_s(&tm_log, &time_log);
	va_list arg;
	va_start(arg, str);
	wstring format1 = FormatString(L"[%02d:%02d:%02d] %s", tm_log.tm_hour, tm_log.tm_min, tm_log.tm_sec, str);
	wstring out = FormatString(format1.c_str(), arg);
	SendMessage(hListBoxStatus, LB_ADDSTRING, 0, (LPARAM)out.c_str());
	SendMessage(hListBoxStatus, LB_SETTOPINDEX, ListBox_GetCount(hListBoxStatus) - 1, 0);
	va_end(arg);
}
void VHookFWindow(const wchar_t* hWndStr) {
	HWND hWnd = (HWND)_wtol(hWndStr);
	if (IsWindow(hWnd)) {
		//GuangBo window fix
		if (hWnd != jiYuGBWnd) {
			WCHAR text[16];
			GetWindowText(hWnd, text, 16);
			if (StrEqual(text, L"屏幕广播") || StrEqual(text, L"屏幕演播室窗口")) {
				VFixGuangBoWindow(hWnd);
				jiYuGBWnd = hWnd;
			}
		}
		if (!VIsInIllegalWindows(hWnd)) {
			jiYuWnds.push_back(hWnd);
		}
	}
}
void VHookWindow(const wchar_t* hWndStr) {
	HWND hWnd = (HWND)_wtol(hWndStr);
	if (IsWindow(hWnd)) {
		//GuangBo window fix
		if (hWnd != jiYuGBWnd) {
			WCHAR text[16];
			GetWindowText(hWnd, text, 16);
			if (StrEqual(text, L"屏幕广播") || StrEqual(text, L"屏幕演播室窗口")) {
				VFixGuangBoWindow(hWnd);
				jiYuGBWnd = hWnd;
			}
		}
		//
		if (!VIsInIllegalWindows(hWnd)) {
			jiYuWnds.push_back(hWnd);
			//jiYuWndCanSize.push_back(hWnd);
		}
	}
}
void VFixGuangBoWindow(HWND hWnd) {
	WNDPROC oldWndProc = (WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC);
	if (oldWndProc != (WNDPROC)JiYuWndProc) {
		jiYuWndProc = (WNDPROC)oldWndProc;
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)JiYuWndProc);
		VOutPutStatus(L"Hooked hWnd %d (0x%08x) WNDPROC", hWnd, hWnd);
		SendMessage(hWnd, WM_SHOWWINDOW, TRUE, FALSE);
	}
}
bool VIsInIllegalWindows(HWND hWnd) {
	list<HWND>::iterator testiterator;
	for (testiterator = jiYuWnds.begin(); testiterator != jiYuWnds.end(); testiterator++)
	{
		if ((*testiterator) == hWnd)
			return true;
	}
	return false;
}
bool VIsInIllegalCanSizeWindows(HWND hWnd) {
	list<HWND>::iterator testiterator;
	for (testiterator = jiYuWndCanSize.begin(); testiterator != jiYuWndCanSize.end(); testiterator++)
	{
		if ((*testiterator) == hWnd)
			return true;
	}
	return false;
}
void VBoom() {
	FreeLibrary(GetModuleHandle(L"ntdll.dll"));
	CHAR*P = 0;
	*P = 0;
}
bool VCheckIsTargetWindow(LPWSTR text) {
	return (StrEqual(text, L"屏幕广播") || StrEqual(text, L"屏幕演播室窗口") || StrEqual(text, L"BlackScreen Window"));
}
void VSendMessageBack(LPCWSTR buff, HWND hDlg) {
	HWND receiveWindow = FindWindow(NULL, L"JY Killer");
	if (receiveWindow) {
		COPYDATASTRUCT copyData = { 0 };
		copyData.lpData = (PVOID)buff;
		copyData.cbData = sizeof(WCHAR) * (wcslen(buff) + 1);
		SendMessageTimeout(receiveWindow, WM_COPYDATA, (WPARAM)hDlg, (LPARAM)&copyData, SMTO_NORMAL, 500, 0);
	}
}
void VManualQuit()
{
	VCloseFuckDrivers();
	VCloseMsgCenter();
	jiYuWnds.clear();
	loaded = false;
}

BOOL hk1 = 0, hk2 = 0, hk3 = 0, hk4 = 0,
hk5 = 0, hk6 = 0, hk7 = 0, hk8 = 0,
hk9 = 0, hk10 = 0, hk11 = 0, hk12 = 0,
hk13 = 0, hk14 = 0, hk15 = 0, hk16 = 0,
hk17 = 0, hk18 = 0, hk19= 0, hk20 = 0;

//Real api address

fnSetWindowPos raSetWindowPos = NULL;
fnMoveWindow raMoveWindow = NULL;
fnSetForegroundWindow raSetForegroundWindow = NULL;
fnBringWindowToTop faBringWindowToTop = NULL;
fnDeviceIoControl raDeviceIoControl = NULL;
fnCreateFileA faCreateFileA = NULL;
fnCreateFileW faCreateFileW = NULL;
fnSetWindowsHookExA faSetWindowsHookExA = NULL;
fnDeferWindowPos faDeferWindowPos = NULL;
fnSendInput faSendInput = NULL;
fnmouse_event famouse_event = NULL;
fnChangeDisplaySettingsW faChangeDisplaySettingsW = NULL;
fnTDDeskCreateInstance faTDDeskCreateInstance = NULL;
fnSetWindowLongA faSetWindowLongA = NULL;
fnSetWindowLongW faSetWindowLongW = NULL;
fnShowWindow faShowWindow = NULL;

void VInstallHooks() {

	//Mhook_SetHook
	HMODULE hUser32 = GetModuleHandle(L"user32.dll");
	HMODULE hKernel32 = GetModuleHandle(L"kernel32.dll");

	raSetWindowPos = (fnSetWindowPos)GetProcAddress(hUser32, "SetWindowPos");
	raMoveWindow = (fnMoveWindow)GetProcAddress(hUser32, "MoveWindow");
	raSetForegroundWindow = (fnSetForegroundWindow)GetProcAddress(hUser32, "SetForegroundWindow");
	faSetWindowsHookExA = (fnSetWindowsHookExA)GetProcAddress(hUser32, "SetWindowsHookExA");
	faDeferWindowPos = (fnDeferWindowPos)GetProcAddress(hUser32, "DeferWindowPos");
	faBringWindowToTop = (fnBringWindowToTop)GetProcAddress(hUser32, "BringWindowToTop");
	faSendInput = (fnSendInput)GetProcAddress(hUser32, "SendInput");
	faChangeDisplaySettingsW = (fnChangeDisplaySettingsW)GetProcAddress(hUser32, "ChangeDisplaySettingsW");
	famouse_event = (fnmouse_event)GetProcAddress(hUser32, "mouse_event");
	faSetWindowLongA = (fnSetWindowLongA)GetProcAddress(hUser32, "SetWindowLongA");
	faSetWindowLongW = (fnSetWindowLongW)GetProcAddress(hUser32, "SetWindowLongW");
	faShowWindow = (fnShowWindow)GetProcAddress(hUser32, "ShowWindow");

	raDeviceIoControl = (fnDeviceIoControl)GetProcAddress(hKernel32, "DeviceIoControl");
	faCreateFileA = (fnCreateFileA)GetProcAddress(hKernel32, "CreateFileA");
	faCreateFileW = (fnCreateFileW)GetProcAddress(hKernel32, "CreateFileW");

	//HMODULE hTDDesk2 = GetModuleHandle(L"libtddesk2.dll");
	//if (hTDDesk2) {
	//	faTDDeskCreateInstance = (fnTDDeskCreateInstance)GetProcAddress(hTDDesk2, "TDDeskCreateInstance");
	//}

	hk1 = Mhook_SetHook((PVOID*)&raSetWindowPos, hkSetWindowPos);
	hk2 = Mhook_SetHook((PVOID*)&raMoveWindow, hkMoveWindow);
	hk3 = Mhook_SetHook((PVOID*)&raSetForegroundWindow, hkSetForegroundWindow);
	hk4 = Mhook_SetHook((PVOID*)&faBringWindowToTop, hkBringWindowToTop);
	hk5 = Mhook_SetHook((PVOID*)&raDeviceIoControl, hkDeviceIoControl);
	hk6 = Mhook_SetHook((PVOID*)&faCreateFileA, hkCreateFileA);
	hk7 = Mhook_SetHook((PVOID*)&faCreateFileW, hkCreateFileW);
	hk8 = Mhook_SetHook((PVOID*)&faSetWindowsHookExA, hkSetWindowsHookExA);
	hk9 = Mhook_SetHook((PVOID*)&faDeferWindowPos, hkDeferWindowPos);
	hk10 = Mhook_SetHook((PVOID*)&faSendInput, hkSendInput);
	hk11 = Mhook_SetHook((PVOID*)&famouse_event, hkmouse_event);

	//if(faTDDeskCreateInstance) 
	//	hk15 = Mhook_SetHook((PVOID*)&faTDDeskCreateInstance, hkTDDeskCreateInstance);

	hk16 = Mhook_SetHook((PVOID*)&faSetWindowLongA, hkSetWindowLongA);
	hk17 = Mhook_SetHook((PVOID*)&faSetWindowLongW, hkSetWindowLongW);
	hk18 = Mhook_SetHook((PVOID*)&faShowWindow, hkShowWindow);

}
void VUnInstallHooks() {

	if (hk1) Mhook_Unhook((PVOID*)&raSetWindowPos);
	if (hk2) Mhook_Unhook((PVOID*)&raMoveWindow);
	if (hk3) Mhook_Unhook((PVOID*)&raSetForegroundWindow);
	if (hk4) Mhook_Unhook((PVOID*)&faBringWindowToTop);
	if (hk5) Mhook_Unhook((PVOID*)&raDeviceIoControl);
	if (hk6) Mhook_Unhook((PVOID*)&faCreateFileA);
	if (hk7) Mhook_Unhook((PVOID*)&faCreateFileW);
	if (hk8) Mhook_Unhook((PVOID*)&faSetWindowsHookExA);
	if (hk9) Mhook_Unhook((PVOID*)&faDeferWindowPos);
	if (hk10) Mhook_Unhook((PVOID*)&faSendInput);
	if (hk11) Mhook_Unhook((PVOID*)&famouse_event);
	//if (hk12) Mhook_Unhook((PVOID*)&faChangeDisplaySettingsW);
	//if (hk13) Mhook_Unhook((PVOID*)&faOpenDesktopA);
	//if (hk14) Mhook_Unhook((PVOID*)&faOpenInputDesktop);
	//if (hk15) Mhook_Unhook((PVOID*)&faTDDeskCreateInstance);
	if (hk16) Mhook_Unhook((PVOID*)&faSetWindowLongA);
	if (hk17) Mhook_Unhook((PVOID*)&faSetWindowLongW);
	if (hk18) Mhook_Unhook((PVOID*)&faShowWindow);

}

//Fuck driver devices
HANDLE hDeviceTDKeybd = NULL;
HANDLE hDeviceTDProcHook = NULL;

void VOpenFuckDrivers() {
	hDeviceTDKeybd = CreateFile(L"\\\\.\\TDKeybd", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	hDeviceTDProcHook = CreateFile(L"\\\\.\\TDProcHook", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}
void VCloseFuckDrivers()
{
	if (hDeviceTDProcHook) CloseHandle(hDeviceTDProcHook);
	if (hDeviceTDKeybd) CloseHandle(hDeviceTDKeybd);
}

//Hook stubs

BOOL WINAPI hkSetForegroundWindow(HWND hWnd)
{
	if (loaded && VIsInIllegalWindows(hWnd))
		return TRUE;
	return raSetForegroundWindow(hWnd);
}
BOOL WINAPI hkSetWindowPos(HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags)
{
	if (loaded) 
	{
		if (x == 0 && y == 0 && cx == screenWidth && cy == screenHeight) 
		{
			if (VIsInIllegalCanSizeWindows(hWnd))
			{
				jiYuWndCanSize.remove(hWnd);
				raSetWindowPos(hWnd, hWndInsertAfter, x, y, cx, cy, uFlags | SWP_NOZORDER);
				SendMessage(hWnd, WM_SIZE, NULL, NULL);
				return TRUE;
			}
			return TRUE;
		}
		if (VIsInIllegalWindows(hWnd)) 
			return TRUE;
	}
	return raSetWindowPos(hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
}
HDWP WINAPI hkDeferWindowPos(HDWP hWinPosInfo, HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags)
{
	if (loaded)
	{
		if (x == 0 && y == 0 && cx == screenWidth && cy == screenHeight) {
			if (VIsInIllegalCanSizeWindows(hWnd)) {
				jiYuWndCanSize.remove(hWnd);
				HDWP rs = faDeferWindowPos(hWinPosInfo, hWnd, hWndInsertAfter, x, y, cx, cy, uFlags | SWP_NOZORDER);
				SendMessage(hWnd, WM_SIZE, NULL, NULL);
				return rs;
			}
			return NULL;
		}
		if (VIsInIllegalWindows(hWnd))
			return NULL;
	}
	return faDeferWindowPos(hWinPosInfo, hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
}
BOOL WINAPI hkMoveWindow(HWND hWnd, int x, int y, int cx, int cy, BOOL bRepaint)
{
	if (loaded)
	{
		if (x == 0 && y == 0 && cx == screenWidth && cy == screenHeight) {
			if (VIsInIllegalCanSizeWindows(hWnd)) {
				jiYuWndCanSize.remove(hWnd);
				return raMoveWindow(hWnd,  x, y, cx, cy, bRepaint);
			}
			return TRUE;
		}
		if (VIsInIllegalWindows(hWnd))
			return TRUE;
	} 
	return raMoveWindow(hWnd, x, y, cx, cy, bRepaint);
}
BOOL WINAPI hkDeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned,  LPOVERLAPPED lpOverlapped) {
	if (hDeviceTDKeybd) {
		if (hDevice == hDeviceTDKeybd) {
			SetLastError(ERROR_ACCESS_DENIED);
			return FALSE;
		}
	}
	if (hDeviceTDProcHook) {
		if (hDevice == hDeviceTDProcHook) {
			SetLastError(ERROR_ACCESS_DENIED);
			return FALSE;
		}
	}
	return raDeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);
}
HANDLE WINAPI hkCreateFileA(LPCSTR lpFileName,  DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile ) {
	if (loaded)
	{
		if (StringHlp::StrEqualA(lpFileName, "\\\\.\\TDKeybd")) {
			SetLastError(ERROR_ACCESS_DENIED);
			return INVALID_HANDLE_VALUE;
		}
		if (StringHlp::StrEqualA(lpFileName, "\\\\.\\TDProcHook")) {
			SetLastError(ERROR_ACCESS_DENIED);
			return INVALID_HANDLE_VALUE;
		}
	}
	return faCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}
HANDLE WINAPI hkCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,  DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
	if (loaded)
	{
		if (StringHlp::StrEqualW(lpFileName, L"\\\\.\\TDKeybd")) {
			SetLastError(ERROR_ACCESS_DENIED);
			return INVALID_HANDLE_VALUE;
		}
		if (StringHlp::StrEqualW(lpFileName, L"\\\\.\\TDProcHook")) {
			SetLastError(ERROR_ACCESS_DENIED);
			return INVALID_HANDLE_VALUE;
		}
	}
	return faCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}
HHOOK WINAPI hkSetWindowsHookExA(int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId)
{
	if (loaded)
	{
		if (idHook == WH_CBT || idHook == WH_MOUSE_LL || idHook == WH_MOUSE) {
			SetLastError(ERROR_ACCESS_DENIED);
			return FALSE;
		}
	}
	return faSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
}
VOID WINAPI hkmouse_event(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData,  ULONG_PTR dwExtraInfo)
{
	
}
UINT WINAPI hkSendInput(UINT cInputs, LPINPUT pInputs, int cbSize)
{
	if (loaded)
		return cInputs;
	return faSendInput(cInputs, pInputs, cbSize);
}
BOOL WINAPI hkBringWindowToTop(_In_ HWND hWnd)
{
	if (loaded)
	{
		if (VIsInIllegalWindows(hWnd))
			return TRUE;
	}
	return faBringWindowToTop(hWnd);
}

LONG WINAPI hkChangeDisplaySettingsW(DEVMODEW* lpDevMode, DWORD dwFlags)
{
	return DISP_CHANGE_SUCCESSFUL;
}
HDESK WINAPI hkOpenDesktopA(LPCSTR lpszDesktop, DWORD dwFlags, BOOL fInherit, ACCESS_MASK dwDesiredAccess)
{
	SetLastError(ERROR_ACCESS_DENIED);
	return NULL;
}
HDESK WINAPI hkOpenInputDesktop(DWORD dwFlags,BOOL fInherit, ACCESS_MASK dwDesiredAccess)
{
	SetLastError(ERROR_ACCESS_DENIED);
	return NULL;
}
HRESULT __cdecl hkTDDeskCreateInstance(CLSID *rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, IID *riid, LPVOID *ppv)
{
	return E_FAIL;
}
LONG WINAPI hkSetWindowLongA(HWND hWnd, int nIndex, LONG dwNewLong)
{
	if (loaded && VIsInIllegalWindows(hWnd))
		return GetWindowLongA(hWnd, nIndex);
	return faSetWindowLongA(hWnd, nIndex, dwNewLong);
}
LONG WINAPI hkSetWindowLongW(HWND hWnd, int nIndex, LONG dwNewLong)
{
	if (loaded && VIsInIllegalWindows(hWnd))
		return GetWindowLongW(hWnd, nIndex);
	return faSetWindowLongW(hWnd, nIndex, dwNewLong);
}
BOOL WINAPI hkShowWindow(HWND hWnd, int nCmdShow)
{
	if (nCmdShow == SW_SHOW || nCmdShow == SW_SHOWNORMAL) {
		WCHAR text[32];
		GetWindowText(hWnd, text, 32);
		if (VCheckIsTargetWindow(text)) {
			VSendMessageBack(L"hkb:immck", hWndMsgCenter);
		}
	}
	return faShowWindow(hWnd, nCmdShow);
}

//HOOK Virus stub
EXTERN_C HRESULT __declspec(dllexport) __cdecl TDAjustCreateInstance(CLSID *rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, IID *riid, LPVOID *ppv)
{
	return faTDAjustCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}

INT_PTR CALLBACK MainWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG: {
		SetWindowText(hDlg, L"JY Killer Virus");
		//SetTimer(hDlg, TIMER_WATCH_DOG_SRV, 10000,	NULL);
		SetTimer(hDlg, TIMER_AUTO_HIDE, 5000, NULL);
		break;
	}
	case WM_DESTROY: {
		//KillTimer(hDlg, TIMER_WATCH_DOG_SRV);
		break;
	}
	case WM_SYSCOMMAND: {
		if (wParam == SC_CLOSE)
			DestroyWindow(hWndMsgCenter);
		break;
	}
	case WM_COMMAND: {
		if (wParam == IDC_KILL) {
			PostQuitMessage(0);
			ExitProcess(0);
		}
		if (wParam == IDC_SMINSIZE) {
			ShowWindow(hDlg, SW_MINIMIZE);
		}
		if (wParam == IDC_SHIDE) {
			RECT rc; GetWindowRect(hDlg, &rc);
			SetWindowPos(hDlg, 0, rc.left, -56, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
		}
		break;
	}
	case WM_COPYDATA: {
		PCOPYDATASTRUCT  pCopyDataStruct = (PCOPYDATASTRUCT)lParam;
		if (pCopyDataStruct->cbData > 0)
		{
			WCHAR recvData[256] = { 0 };
			wcsncpy_s(recvData, (WCHAR *)pCopyDataStruct->lpData, pCopyDataStruct->cbData);
			VHandleMsg(recvData);
		}
		break;
	}
	case WM_LBUTTONDOWN: {
		RECT rc; GetWindowRect(hDlg, &rc);
		if (rc.top == -56) {
			SetWindowPos(hDlg, 0, rc.left, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
		}
		else {
			ReleaseCapture();
			SendMessage(hDlg, WM_NCLBUTTONDOWN, HTCAPTION, 0);
		}
		break;
	}
	case WM_TIMER: {
		if (wParam == TIMER_AUTO_HIDE) {
			KillTimer(hDlg, TIMER_AUTO_HIDE);
			SendMessage(hDlg, WM_COMMAND, IDC_SHIDE, NULL);
		}
		break;
	}
	default:
		break;
	}
	return 0;
}
INT_PTR CALLBACK JiYuWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	/*
	if (message == WM_TIMER) {
		KillTimer(hWnd, wParam);
		return 0;
	}
	*/
	if (message == WM_GETMINMAXINFO) {
		PMINMAXINFO pMinMaxInfo = (PMINMAXINFO)lParam;
		pMinMaxInfo->ptMinTrackSize.x = 0;
		pMinMaxInfo->ptMinTrackSize.y = 0;
		return 0;
	}
	if (message == WM_SIZE) {
		RECT rcWindow;
		RECT rcClient;
		GetWindowRect(hWnd, &rcWindow);
		GetClientRect(hWnd, &rcClient);

		bool setToFull = false;
		if (rcWindow.right - rcWindow.left == screenWidth && rcWindow.bottom - rcWindow.top == screenHeight)
			setToFull = true;
		if (jiYuGBDeskRdWnd == NULL) {
			jiYuGBDeskRdWnd = FindWindowExW(hWnd, NULL, NULL, L"TDDesk Render Window");
			//HOOK TDDesk Render Window for WM_SIZE
			WNDPROC oldWndProc = (WNDPROC)GetWindowLong(jiYuGBDeskRdWnd, GWL_WNDPROC);
			if (oldWndProc != (WNDPROC)JiYuTDDeskWndProc) {
				jiYuTDDeskWndProc = (WNDPROC)oldWndProc;
				SetWindowLong(jiYuGBDeskRdWnd, GWL_WNDPROC, (LONG)JiYuTDDeskWndProc);
				VOutPutStatus(L"Hooked jiYuGBDeskRdWnd %d (0x%08x) WNDPROC", jiYuGBDeskRdWnd, jiYuGBDeskRdWnd);
			}
		}
		if (!IsWindow(jiYuGBDeskRdWnd) || GetParent(jiYuGBDeskRdWnd) != hWnd)
			jiYuGBDeskRdWnd = NULL;
		if (jiYuGBDeskRdWnd != NULL)
			if (setToFull) MoveWindow(jiYuGBDeskRdWnd, 0, 0, screenWidth, screenHeight, TRUE);
			else {
				SendMessage(jiYuGBDeskRdWnd, WM_SIZE, 0, MAKELPARAM(10, 10));
				//MoveWindow(jiYuGBDeskRdWnd, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, TRUE);
			}
	}
	if (message == WM_SHOWWINDOW)
	{
		if (wParam)
		{
			SendMessage(hWnd, WM_SIZE, NULL, NULL);
			int w = (int)((double)screenWidth * (3.0 / 4.0)), h = (int)((double)screenHeight * (double)(4.0 / 5.0));
			if (raSetWindowPos) raSetWindowPos(hWnd, 0, (screenWidth - w) / 2, (screenHeight - h) / 2, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
			else SetWindowPos(hWnd, 0, (screenWidth - w) / 2, (screenHeight - h) / 2, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		else jiYuWndCanSize.push_back(hWnd);
	}
	if (message == WM_DESTROY)
	{
		jiYuWnds.remove(hWnd);
		jiYuWndCanSize.remove(hWnd);
	}

	if (jiYuWndProc) return jiYuWndProc(hWnd, message, wParam, lParam);
	else return DefWindowProc(hWnd, message, wParam, lParam);
}
INT_PTR CALLBACK JiYuTDDeskWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_SIZE) {
		RECT rcParent;
		GetClientRect(GetParent(hWnd), &rcParent);
		int w = LOWORD(lParam), h = HIWORD(lParam),
			rw = rcParent.right - rcParent.left, rh = rcParent.bottom - rcParent.top;
		if (w != rw || h != rh) 
			raMoveWindow(hWnd, 0, 0, rw, rh, TRUE);
	}
	if (jiYuTDDeskWndProc) return jiYuTDDeskWndProc(hWnd, message, wParam, lParam);
	else return DefWindowProc(hWnd, message, wParam, lParam);
}

