#pragma once
#include "stdafx.h"

LPWSTR WGetCkStatText();

bool WInitResolver();
void WUnInitResolver();
int WRunCk();

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);

bool WChecIsJIYuWindow(HWND hWnd, LPDWORD outPid, LPDWORD outTid);
bool WCheckIsTargetWindow(LPWSTR text);
void WFixWindow(HWND hWnd, LPWSTR text);
