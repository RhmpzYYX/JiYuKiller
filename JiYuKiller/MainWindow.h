#pragma once
#include "stdafx.h"

bool XInitApp();
void XQuitApp();

bool XPreReadCommandLine(LPWSTR * szArgList, int argCount);
void XOutPutStatus(const wchar_t * str, ...);
void XLoadConfig();
ATOM XRegisterClass(HINSTANCE hInstance);
INT XCheckRunningApp();

void SetCK(bool enable);
void ResetPid();

void SetProcStatus(bool ctled, LPWSTR extendInfo);
void SetCtlStatus(bool ctled, LPWSTR extendInfo);
void SetCkStatus(int s, LPWSTR str);

void UpdateLogoState();
void SetCkEnd();
void CreateTrayIcon(HWND hDlg);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void OnSetOutPutStat(LPWSTR str);
