#pragma once
#include "stdafx.h"

VOID CALLBACK MainTimerProc(HWND hwnd, UINT message, UINT iTimerID, DWORD dwTime);

bool XInitApp();
void XQuitApp();

bool XPreReadCommandLine(LPWSTR * szArgList, int argCount);
void XOutPutStatus(const wchar_t * str, ...);

void XLoadConfig();

ATOM XRegisterClass(HINSTANCE hInstance);


void SetCK(bool enable);

void ResetPid();

void ResetCtl();

void SetProcStatus(bool ctled, LPWSTR extendInfo);
void SetCtlStatus(bool ctled, LPWSTR extendInfo);
void SetCkStatus(int s, LPWSTR str);

void CreateTrayIcon(HWND hDlg);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void OnSetOutPutStat(LPWSTR str);
