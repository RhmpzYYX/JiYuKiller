#pragma once
#include "stdafx.h"
#include <unknwn.h>

void VUnloadAll();
void VLoad();
void VRunMain();

void VLoadRealVirus();

void VParamInit();

void VCreateMsgCenter();
DWORD WINAPI VMsgCenterRunThread(LPVOID lpThreadParameter);
void VCloseMsgCenter();

void VOutPutStatus(const wchar_t * str, ...);

void VHookFWindow(const wchar_t * hWndStr);

void VHookWindow(const wchar_t * hWndStr);
void VFixGuangBoWindow(HWND hWnd);
bool VIsInIllegalWindows(HWND hWnd);
void VBoom();

bool VCheckIsTargetWindow(LPWSTR text);

void VSendMessageBack(LPCWSTR buff, HWND hDlg);

void VManualQuit();

void VInstallHooks();
void VUnInstallHooks();

void VOpenFuckDrivers();

void VCloseFuckDrivers();

INT_PTR CALLBACK MainWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK JiYuWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK JiYuTDDeskWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

typedef BOOL (WINAPI *fnSetForegroundWindow)(HWND hWnd);
typedef BOOL (WINAPI *fnSetWindowPos)(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
typedef BOOL (WINAPI *fnMoveWindow)(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);
typedef BOOL (WINAPI *fnBringWindowToTop)(HWND hWnd); 
typedef BOOL (WINAPI *fnDeviceIoControl)(
	__in        HANDLE hDevice,
	__in        DWORD dwIoControlCode,
	__in_bcount_opt(nInBufferSize) LPVOID lpInBuffer,
	__in        DWORD nInBufferSize,
	__out_bcount_part_opt(nOutBufferSize, *lpBytesReturned) LPVOID lpOutBuffer,
	__in        DWORD nOutBufferSize,
	__out_opt   LPDWORD lpBytesReturned,
	__inout_opt LPOVERLAPPED lpOverlapped
);
typedef HANDLE (WINAPI *fnCreateFileA)(
	__in     LPCSTR lpFileName,
	__in     DWORD dwDesiredAccess,
	__in     DWORD dwShareMode,
	__in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	__in     DWORD dwCreationDisposition,
	__in     DWORD dwFlagsAndAttributes,
	__in_opt HANDLE hTemplateFile
);
typedef HANDLE(WINAPI *fnCreateFileW)(
	__in     LPCWSTR lpFileName,
	__in     DWORD dwDesiredAccess,
	__in     DWORD dwShareMode,
	__in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	__in     DWORD dwCreationDisposition,
	__in     DWORD dwFlagsAndAttributes,
	__in_opt HANDLE hTemplateFile
);
typedef HHOOK (WINAPI *fnSetWindowsHookExA)(
	__in int idHook,
	__in HOOKPROC lpfn,
	__in_opt HINSTANCE hmod,
	__in DWORD dwThreadId);
typedef HDWP (WINAPI *fnDeferWindowPos)(
	_In_ HDWP hWinPosInfo,
	_In_ HWND hWnd,
	_In_opt_ HWND hWndInsertAfter,
	_In_ int x,
	_In_ int y,
	_In_ int cx,
	_In_ int cy,
	_In_ UINT uFlags);
typedef VOID(WINAPI *fnmouse_event)(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo);
typedef UINT (WINAPI *fnSendInput)(
	__in UINT cInputs,                     // number of input in the array
	__in_ecount(cInputs) LPINPUT pInputs,  // array of inputs
	__in int cbSize);
typedef BOOL(WINAPI *fnSetThreadDesktop)(__in HDESK hDesktop);
typedef LONG(WINAPI *fnChangeDisplaySettingsW)(
	__in_opt DEVMODEW* lpDevMode,
	__in DWORD dwFlags);
typedef HDESK (WINAPI  *fnOpenDesktopA)(
	__in LPCSTR lpszDesktop,
	__in DWORD dwFlags,
	__in BOOL fInherit,
	__in ACCESS_MASK dwDesiredAccess);
typedef HDESK (WINAPI  *fnOpenInputDesktop)(
	__in DWORD dwFlags,
	__in BOOL fInherit,
	__in ACCESS_MASK dwDesiredAccess);
typedef HRESULT(__cdecl *fnTDDeskCreateInstance)(CLSID *rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, IID *riid, LPVOID *ppv);
typedef LONG (WINAPI  *fnSetWindowLongA)(
	__in HWND hWnd,
	__in int nIndex,
	__in LONG dwNewLong);
typedef LONG (WINAPI  *fnSetWindowLongW)(
	__in HWND hWnd,
	__in int nIndex,
	__in LONG dwNewLong);
typedef HRESULT(__cdecl *fnTDAjustCreateInstance)(CLSID *rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, IID *riid, LPVOID *ppv);
typedef BOOL (WINAPI *fnShowWindow)(
	_In_ HWND hWnd,
	_In_ int nCmdShow);


//Hooks

BOOL WINAPI hkSetWindowPos(HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags);
BOOL WINAPI hkMoveWindow(HWND hWnd, int x, int y, int cx, int cy, BOOL bRepaint);
BOOL WINAPI hkSetForegroundWindow(HWND hWnd);
BOOL WINAPI hkBringWindowToTop(_In_ HWND hWnd);
BOOL WINAPI hkDeviceIoControl(
	__in        HANDLE hDevice,
	__in        DWORD dwIoControlCode,
	__in_bcount_opt(nInBufferSize) LPVOID lpInBuffer,
	__in        DWORD nInBufferSize,
	__out_bcount_part_opt(nOutBufferSize, *lpBytesReturned) LPVOID lpOutBuffer,
	__in        DWORD nOutBufferSize,
	__out_opt   LPDWORD lpBytesReturned,
	__inout_opt LPOVERLAPPED lpOverlapped
	);
HANDLE WINAPI hkCreateFileA(
	__in     LPCSTR lpFileName,
	__in     DWORD dwDesiredAccess,
	__in     DWORD dwShareMode,
	__in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	__in     DWORD dwCreationDisposition,
	__in     DWORD dwFlagsAndAttributes,
	__in_opt HANDLE hTemplateFile
);
HANDLE WINAPI hkCreateFileW(
	__in     LPCWSTR lpFileName,
	__in     DWORD dwDesiredAccess,
	__in     DWORD dwShareMode,
	__in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	__in     DWORD dwCreationDisposition,
	__in     DWORD dwFlagsAndAttributes,
	__in_opt HANDLE hTemplateFile
);
HHOOK WINAPI hkSetWindowsHookExA(
	__in int idHook,
	__in HOOKPROC lpfn,
	__in_opt HINSTANCE hmod,
	__in DWORD dwThreadId);
HDWP WINAPI hkDeferWindowPos(
	_In_ HDWP hWinPosInfo,
	_In_ HWND hWnd,
	_In_opt_ HWND hWndInsertAfter,
	_In_ int x,
	_In_ int y,
	_In_ int cx,
	_In_ int cy,
	_In_ UINT uFlags);
UINT WINAPI hkSendInput(
	__in UINT cInputs,                     // number of input in the array
	__in_ecount(cInputs) LPINPUT pInputs,  // array of inputs
	__in int cbSize);
VOID WINAPI hkmouse_event(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo);
LONG WINAPI hkChangeDisplaySettingsW(
	__in_opt DEVMODEW* lpDevMode,
	__in DWORD dwFlags);
HDESK WINAPI hkOpenDesktopA(
	__in LPCSTR lpszDesktop,
	__in DWORD dwFlags,
	__in BOOL fInherit,
	__in ACCESS_MASK dwDesiredAccess);
HDESK WINAPI hkOpenInputDesktop(
	__in DWORD dwFlags,
	__in BOOL fInherit,
	__in ACCESS_MASK dwDesiredAccess);
LONG WINAPI hkSetWindowLongA(
	__in HWND hWnd,
	__in int nIndex,
	__in LONG dwNewLong);
LONG WINAPI hkSetWindowLongW(
	__in HWND hWnd,
	__in int nIndex,
	__in LONG dwNewLong);
BOOL WINAPI hkShowWindow(
	_In_ HWND hWnd,
	_In_ int nCmdShow);

HRESULT __cdecl hkTDDeskCreateInstance(CLSID *rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, IID *riid, LPVOID *ppv);

typedef struct tag_WNDIJJ {
	HWND wnd;
	bool canSizeToFull;
}WNDIJJ,*PWNDIJJ;