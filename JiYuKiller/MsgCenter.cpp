#include "stdafx.h"
#include "MsgCenter.h"


void MsgCenterSendToVirus(LPWSTR buff, HWND form) 
{
	HWND receiveWindow = FindWindow(NULL, L"JY Killer Virus");
	if (receiveWindow) {
		COPYDATASTRUCT copyData = { 0 };
		copyData.lpData = buff;
		copyData.cbData = sizeof(WCHAR) * (wcslen(buff) + 1);
		SendMessageTimeout(receiveWindow, WM_COPYDATA, (WPARAM)form, (LPARAM)&copyData, SMTO_ABORTIFHUNG | SMTO_NORMAL, 500, 0);
	}
}