#pragma once
#include "resource.h"

void Loader_GenAllPath(int o);
bool Loader_IsUsbDrv(const wchar_t *path);
bool Loader_CheckAndInstall(const wchar_t *path);
bool Loader_ExtractFile(int res_id, const wchar_t * to_path);
int Loader_RunMain();


