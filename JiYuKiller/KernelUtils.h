#pragma once
#include "stdafx.h"
#include "NtHlp.h"

bool KFShutdown();
bool KForceKill(DWORD pid, NTSTATUS * pStatus);