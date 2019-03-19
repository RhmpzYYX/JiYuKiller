#pragma once
#include  "stdafx.h"
#include "NtHlp.h"

bool TryFindStudent(DWORD * outPid);
bool TryFindXX(DWORD * outPid, const wchar_t* name);

bool KillSt();

void HungSt();

void ResuseSt();

void FShutdownn();

bool TryFIndStPath();

bool KillStReal();

bool SuspendNormal(DWORD pid, NTSTATUS * pStatus);

bool SuspendIt(DWORD pid, NTSTATUS * pStatus);

bool ReusemeIt(DWORD pid, NTSTATUS * pStatus);

bool KillNormal(DWORD pid);
bool KillKernelSafe(DWORD pid, NTSTATUS *pStatus);
bool KillKernelUnsafe(DWORD pid, NTSTATUS *pStatus);

void ForceUnTopWindow(HWND hWnd);

bool FixJIYuWindow(HWND hWnd, LPWSTR name);
bool CheckIsJIYuWindow(HWND hWnd, LPDWORD outPid = NULL, LPDWORD outTid = NULL);
HWND TryGetJIYuFullscreenWindow();
void RunTopWindowCheckWk();
bool CheckIsTargetWindow(LPWSTR text);
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
void FuckWindow(HWND hWnd, LPWSTR name);

void InstallVirus();
int EnableDebugPriv(const wchar_t * name);
BOOL InjectDll(const wchar_t * DllFullPath, const DWORD dwRemoteProcessId);

void SendVBoom();

void SendVActive();

void InstallDllHook();

void ExtractParts();

bool WriteResourceToFile(int rsId, LPWSTR toFile);

void TryForceUnloadJiYuDriver();

BOOL InstallMouseHook();
BOOL UninstallMouseHook();

LRESULT CALLBACK LowLevelMouseProc(INT nCode, WPARAM wParam, LPARAM lParam);
