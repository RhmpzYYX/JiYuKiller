#pragma once
#include "stdafx.h"
#include "NtHlp.h"

bool KFShutdown();
bool KFReboot();
bool KForceKill(DWORD pid, NTSTATUS * pStatus);