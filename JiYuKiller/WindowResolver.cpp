#include "stdafx.h"
#include "WindowResolver.h"
#include "Utils.h"
#include "StringHlp.h"
#include "MsgCenter.h"

HDESK hDesktop = NULL;

int screenWidth, screenHeight;
bool lastHasGb = false, lastHasHp = false;
int lastResolveWnd = 0;

#define WS_EX_SIZED 0x10000000L

bool setAutoIncludeFullWindow = false;

WCHAR ckStatText[36];

extern DWORD jiyuPid;

bool WLastState() { return lastHasGb || lastHasHp; }
LPWSTR WGetCkStatText() {
	if (jiyuPid <= 4) return (LPWSTR)L"未发现极域进程";
	else if (!lastHasGb && !lastHasHp) return (LPWSTR)L"未发现极域的非法窗口";
	else return ckStatText;
}
bool WInitResolver() 
{
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	hDesktop = OpenDesktop(L"Default", 0, FALSE, DESKTOP_ENUMERATE);

	return true;
}
void WUnInitResolver() {
	CloseDesktop(hDesktop);
}
int WRunCk() 
{
	lastResolveWnd = 0;
	lastHasGb = false;
	lastHasHp = false;

	EnumDesktopWindows(hDesktop, EnumWindowsProc, 0);
	MsgCenterSendHWNDS();

	if (lastHasGb && lastHasHp) {
		swprintf_s(ckStatText, L"已处理极域非法广播窗口和黑屏窗口，数量：%d", lastResolveWnd);
		return 1;
	}
	else if (lastHasGb) {
		swprintf_s(ckStatText, L"已处理极域非法广播窗口，数量：%d", lastResolveWnd);
		return 1;
	}
	else if (lastHasHp) {
		swprintf_s(ckStatText, L"已处理极域非法黑屏窗口，数量：%d", lastResolveWnd);
		return 1;
	}
	else  return 0;
}

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	if (IsWindowVisible(hWnd) && WChecIsJIYuWindow(hWnd, NULL, NULL)) {
		WCHAR text[32];
		GetWindowText(hWnd, text, 32);
		if (StrEqual(text, L"JY Killer Virus")) return TRUE;

		RECT rc;
		GetWindowRect(hWnd, &rc);
		if (WCheckIsTargetWindow(text)) {
			//JiYu window
			MsgCenteAppendHWND(hWnd);
			WFixWindow(hWnd, text);
		}
		else if (setAutoIncludeFullWindow && rc.top == 0 && rc.left == 0 && rc.right == screenWidth && rc.bottom == screenHeight) {
			//Full window
			MsgCenteAppendHWND(hWnd);
			WFixWindow(hWnd, text);
		}
	}
	return TRUE;
}

bool WChecIsJIYuWindow(HWND hWnd, LPDWORD outPid, LPDWORD outTid) {
	if (jiyuPid == 0) return false;
	DWORD pid = 0, tid = GetWindowThreadProcessId(hWnd, &pid);
	if (outPid) *outPid = pid;
	if (outTid) *outTid = tid;
	return pid == jiyuPid;
}
bool WCheckIsTargetWindow(LPWSTR text) {
	bool b = false;
	if (StrEqual(text, L"屏幕广播") || StrEqual(text, L"屏幕演播室窗口")) {
		b = true;
		lastHasGb = true;
	}
	if (StrEqual(text, L"BlackScreen Window")) {
		b = true;
		lastHasHp = true;
	}
	return b;
}
void WFixWindow(HWND hWnd, LPWSTR text)
{
	lastResolveWnd++;
	//Un top
	LONG oldLong = GetWindowLong(hWnd, GWL_EXSTYLE);
	if ((oldLong & WS_EX_TOPMOST) == WS_EX_TOPMOST)
	{
		SetWindowLong(hWnd, GWL_EXSTYLE, oldLong ^ WS_EX_TOPMOST);
		SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	}
	//Set border and sizeable
	SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) | (WS_BORDER | WS_OVERLAPPEDWINDOW));
	
	if (StrEqual(text, L"BlackScreen Window")) 
	{
		oldLong = GetWindowLong(hWnd, GWL_EXSTYLE);
		if ((oldLong & WS_EX_NOACTIVATE) != WS_EX_NOACTIVATE)
		{
			SetWindowLong(hWnd, GWL_EXSTYLE, oldLong | WS_EX_NOACTIVATE);
			SetWindowPos(hWnd, 0, 20, 20, 90, 150, SWP_NOZORDER | SWP_DRAWFRAME | SWP_NOACTIVATE);
		}
	}

	SetWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_DRAWFRAME | SWP_NOACTIVATE);
}