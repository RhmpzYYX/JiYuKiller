#pragma once
#include "stdafx.h"

LPWSTR TGetLastError();

DWORD TGetLastJiYuPid();
LPWSTR TGetLastJiYuPayh();
bool TFindTarget(LPDWORD outPid, bool *isNewState);

bool TLocated();

LPWSTR TGetCurrStatText();

LPWSTR TGetCurrProcStatText();

bool TReset();
int TRunCK();
void TSendCtlStat();
void TSendCkEnd();
void TSendBoom();
void TFindJiYuPath();
bool TFirstStartUp();


bool TFindJiYuInstallLocation(bool usePid);


bool TInstallVirus();
bool TInstallVirusWithRemoteThread();
bool TInstallVirusWithDllApi();


bool TForceKill();