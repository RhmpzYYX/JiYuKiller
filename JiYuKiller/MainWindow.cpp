#include "stdafx.h"
#include "MainWindow.h"
#include "Resource.h"
#include "WindowResolver.h"
#include "AboutWindow.h"
#include "Tracker.h"
#include "DriverLoader.h"
#include "htmlayout.h"
#include "NtHlp.h"
#include "KernelUtils.h"
#include "StringHlp.h"
#include "StringSplit.h"
#include <shlwapi.h>
#include <time.h>
#include <string>
#include "JiYuKiller.h"

using namespace std;

extern int screenWidth, screenHeight;
extern HINSTANCE hInst;
extern WCHAR currentDir[MAX_PATH];
extern WCHAR fullPath[MAX_PATH];
extern WCHAR iniPath[MAX_PATH];

//sets

extern bool setAutoForceKill;
extern bool setAutoIncludeFullWindow;
int setCKinterval = 4000;

WCHAR statusBuffer[2048] = { 0 };

bool firstShow = true, fullWindow = false, controlled = false, ck = true, top = false, isUserCancel = false;
HWND hWndMain = NULL, hWndTool = NULL;

#define TIMER_AOP 2
#define TIMER_RESET_PID 3
#define TIMER_TOP_CHECK 4
#define TIMER_CK_DEALY_HIDE 5

bool XCreateMainWindow()
{
	if (XRegisterClass(hInst))
	{
		hWndMain = CreateWindowW(L"XObserver", L"JY Killer", WS_OVERLAPPED |WS_CAPTION | WS_SYSMENU |WS_MINIMIZEBOX,
			CW_USEDEFAULT, 0, 500, 210, nullptr, nullptr, hInst, nullptr);

		if (!hWndMain) return FALSE;

		ShowWindow(hWndMain, SW_SHOW);
		UpdateWindow(hWndMain);

		return TRUE;
	}

	return false;
}
void XDestroyMainWindow()
{
	if (IsWindow(hWndMain))
	{
		DestroyWindow(hWndMain);
		hWndMain = NULL;
	}
}

bool XInitApp()
{
	LoadNt();

	XLoadDriver();
	XLoadConfig();

	if(!WInitResolver())
		return false;
	if (!XCreateMainWindow())
		return false;

	return true;
}
void XQuitApp() 
{
	XUnLoadDriver();
	TSendCkEnd();
	WUnInitResolver();
	XDestroyMainWindow();
	PostQuitMessage(0);
}
bool XPreReadCommandLine(LPWSTR *szArgList, int argCount) {
	if (argCount >= 2) {
		if (wcscmp(szArgList[1], L"rex1") == 0) 
			MessageBox(NULL, L"刚才极域进行了非常严重的非法操纵，企图威胁本程序，现在极域已经被结束。", L"提示", MB_ICONINFORMATION);
		else if (wcscmp(szArgList[1], L"rex2") == 0) {

		}
	}
	return false;
}
void XOutPutStatus(const wchar_t* str, ...)
{
	time_t time_log = time(NULL);
	struct tm tm_log;
	localtime_s(&tm_log, &time_log);
	va_list arg;
	va_start(arg, str);
	wstring format1 = FormatString(L"[%02d:%02d:%02d] %s\n", tm_log.tm_hour, tm_log.tm_min, tm_log.tm_sec, str);
	wstring out = FormatString(format1.c_str(), arg);
	//SendMessage(hListBoxStatus, LB_ADDSTRING, 0, (LPARAM)out.c_str());
	//SendMessage(hListBoxStatus, LB_SETTOPINDEX, ListBox_GetCount(hListBoxStatus) - 1, 0);

	OnSetOutPutStat((LPWSTR)out.c_str());
	OutputDebugString(out.c_str());

	va_end(arg);
}
void XLoadConfig()
{
	if (PathFileExists(iniPath))
	{
		WCHAR w[32];
		GetPrivateProfileString(L"JYK", L"AutoForceKill", L"FALSE", w, 32, iniPath);
		if (StrEqual(w, L"TRUE") || StrEqual(w, L"true") || StrEqual(w, L"1")) setAutoForceKill = true;

		GetPrivateProfileString(L"JYK", L"AutoIncludeFullWindow", L"FALSE", w, 32, iniPath);
		if (StrEqual(w, L"TRUE") || StrEqual(w, L"true") || StrEqual(w, L"1")) setAutoIncludeFullWindow = true;

		GetPrivateProfileString(L"JYK", L"Top", L"FALSE", w, 32, iniPath);
		if (StrEqual(w, L"TRUE") || StrEqual(w, L"true") || StrEqual(w, L"1")) top = true;

		GetPrivateProfileString(L"JYK", L"CKinterval", L"FALSE", w, 32, iniPath);
		if (!StrEmepty(w)) {
			int ww = _wtoi(w);
			if (ww < 1 || ww > 10)
			{
				XOutPutStatus(L"CKinterval 输入了一个无效的参数。CKinterval 有效值为 1-10 [%d]", ww);
				setCKinterval = 4000;
			}
			else setCKinterval = ww * 1000;
		}
	}
	else XOutPutStatus(L"未找到配置文件 [%s]，使用默认配置", iniPath);
}

HMENU hMenuTray;
NOTIFYICONDATA nid;
UINT WM_TASKBARCREATED;

ATOM XRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_JIYUKILLER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"XObserver";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	return RegisterClassExW(&wcex);
}

bool mainDomReday = false;

htmlayout::dom::element stat_ctl_text;
htmlayout::dom::element stat_ctl_text2;
htmlayout::dom::element stat_ico_ctl_no;
htmlayout::dom::element stat_ico_ctl_yes;

htmlayout::dom::element stat_ck_text;
htmlayout::dom::element stat_ico_ck_grey;
htmlayout::dom::element stat_ico_ck_red;
htmlayout::dom::element stat_ico_ck_green;

htmlayout::dom::element exten_area;
htmlayout::dom::element status_area;

htmlayout::dom::element input_cmd;
htmlayout::dom::element link_more;

htmlayout::dom::element stat_ico_proc_red;
htmlayout::dom::element stat_ico_proc_green;
htmlayout::dom::element stat_proc_text;

BOOL OnWmCreate(HWND hWnd)
{
	//Tray icon
	WM_TASKBARCREATED = RegisterWindowMessage(TEXT("TaskbarCreated"));
	CreateTrayIcon(hWnd);
	hMenuTray = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MAINMENU));
	hMenuTray = GetSubMenu(hMenuTray, 0);

	if (top) {
		SetWindowPos(hWndMain, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		SetTimer(hWndMain, TIMER_AOP, 100, NULL);
	}
	SetTimer(hWnd, TIMER_RESET_PID, 5000, NULL);
	ResetPid();
	TFindJiYuPath();
	SetCK(true);

	return TRUE;
}
void OnWmDestroy(HWND hWnd)
{
	KillTimer(hWnd, TIMER_TOP_CHECK);
	KillTimer(hWnd, TIMER_RESET_PID);

	if (!isUserCancel && controlled)
	{
		TSendBoom();
		PostQuitMessage(0);
		ShellExecute(NULL, L"runas", fullPath, L"rex1", NULL, SW_SHOW);
	}
}
void OnDocumentComplete() 
{
	htmlayout::dom::root_element root(hWndMain);
	stat_ctl_text = root.get_element_by_id(L"stat_ctl_text");
	stat_ctl_text2 = root.get_element_by_id(L"stat_ctl_text2");
	stat_ico_ctl_no = root.get_element_by_id(L"stat_ico_ctl_no");
	stat_ico_ctl_yes = root.get_element_by_id(L"stat_ico_ctl_yes");
	stat_ck_text = root.get_element_by_id(L"stat_ck_text");
	stat_ico_ck_grey = root.get_element_by_id(L"stat_ico_ck_grey");
	stat_ico_ck_red = root.get_element_by_id(L"stat_ico_ck_red");
	stat_ico_ck_green = root.get_element_by_id(L"stat_ico_ck_green");
	stat_proc_text = root.get_element_by_id(L"stat_proc_text");
	stat_ico_proc_red = root.get_element_by_id(L"stat_ico_proc_red");
	stat_ico_proc_green = root.get_element_by_id(L"stat_ico_proc_green");

	exten_area = root.get_element_by_id(L"exten_area");
	status_area = root.get_element_by_id(L"status_area");
	input_cmd = root.get_element_by_id(L"input_cmd");

	link_more = root.get_element_by_id(L"link_more");

	if (setAutoForceKill) {
		htmlayout::dom::element ele(root.get_element_by_id(L"check_auto_fkill"));
		htmlayout::set_checkbox_bits(ele, json::value(true));
	}
	if (setAutoIncludeFullWindow) {
		htmlayout::dom::element ele(root.get_element_by_id(L"check_auto_fck"));
		htmlayout::set_checkbox_bits(ele, json::value(true));
	}
	if (top) {
		htmlayout::dom::element ele(root.get_element_by_id(L"check_top"));
		htmlayout::set_checkbox_bits(ele, json::value(true));
	}
	mainDomReday = true;
}
void OnRunCmd(vector<wstring> * cmds, int len)
{
	bool succ = true;
	wstring cmd = (*cmds)[0];
	if (cmd == L"killst") {
		if (TForceKill()) {
			SetCtlStatus(false, (LPWSTR)L"已成功结束极域进程");
			XOutPutStatus(L"已成功结束极域进程");
		}
		else XOutPutStatus(L"无法结束极域进程 %s", TGetLastError());
	}
	else if (cmd == L"rerunst") {
		LPWSTR jiyuPath = TGetLastJiYuPayh();
		if (StrEqual(jiyuPath, L"") || !PathFileExists(jiyuPath)) {
			XOutPutStatus(L"无法定位极域主进程位置 %s", jiyuPath);
			MessageBox(hWndMain, L"无法定位极域主进程位置，请手动启动。", L"错误", 0);
		}
		else {
			ShellExecute(hWndMain, L"open", jiyuPath, NULL, NULL, SW_SHOW);
			XOutPutStatus(L"已经启动极域 %s", jiyuPath);
		}
	}
	else if (cmd == L"ss")  TSendBoom();
	else if (cmd == L"sss") system("shutdown -s -t 0");
	else if (cmd == L"ssss") KFShutdown();
	else if (cmd == L"ckend") TSendCkEnd();
	else {
		succ = false;
		MessageBox(0, L"未知命令", L"JY Killer", 0);
	}
	if (succ) htmlayout::set_value(input_cmd, json::value(L""));
}
void OnWmCommand(HWND hWnd, WPARAM wmId)
{
	switch (wmId)
	{
	case IDM_SHOWMAIN: {
		if (IsWindowVisible(hWnd))
			ShowWindow(hWnd, SW_HIDE);
		else
		{
			ShowWindow(hWnd, SW_SHOW);
			SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		}
		break;
	}
	case IDM_EXIT: SendMessage(hWndMain, WM_SYSCOMMAND, SC_CLOSE, 0); break;
	case IDM_HELP: ARunAboutWindow(hWnd); break;
	default: break;
	}
}
void OnSetOutPutStat(LPWSTR str)
{
	if (wcslen(statusBuffer) < 2000) wcscat_s(statusBuffer, str);
	else wcscpy_s(statusBuffer, str);

	if (mainDomReday) {
		status_area.set_value(json::value(statusBuffer));

		POINT scroll_pos;
		RECT view_rect;
		SIZE content_size;
		status_area.get_scroll_info(scroll_pos, view_rect, content_size);
		scroll_pos.y = content_size.cy;
		status_area.set_scroll_pos(scroll_pos);
	}
}
void OnButtonClick(HELEMENT btn)
{
	htmlayout::dom::element cBut(btn);
	if (StrEqual(cBut.get_attribute("id"), L"check_auto_fck"))
	{
		if (setAutoIncludeFullWindow)
		{
			setAutoIncludeFullWindow = false;
			cBut.remove_attribute("checked");
		}
		else
		{
			setAutoIncludeFullWindow = true;
			cBut.set_attribute("checked", L"checked");
		}
	}
	else if (StrEqual(cBut.get_attribute("id"), L"check_auto_fkill"))
	{
		if (setAutoForceKill)
		{
			setAutoForceKill = false;
			cBut.remove_attribute("checked");
		}
		else
		{
			setAutoForceKill = true;
			cBut.set_attribute("checked", L"checked");
		}
	}
	else if (StrEqual(cBut.get_attribute("id"), L"check_ck"))
	{
		if (ck) 
		{
			ck = false;
			cBut.remove_attribute("checked");
		}
		else
		{
			ck = true;
			cBut.set_attribute("checked", L"checked");
		}
		SetCK(ck);
	}
	else if (StrEqual(cBut.get_attribute("id"), L"check_top"))
	{
		if (top)
		{
			top = false;
			cBut.remove_attribute("checked");

			SetWindowPos(hWndMain, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			KillTimer(hWndMain, TIMER_AOP);
		}
		else
		{
			top = true;
			cBut.set_attribute("checked", L"checked");

			SetWindowPos(hWndMain, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			SetTimer(hWndMain, TIMER_AOP, 100, NULL);
		}
	}
}
void OnLinkClick(HELEMENT link)
{
	htmlayout::dom::element cBut = link;
	if (StrEqual(cBut.get_attribute("id"), L"bottom_about")) SendMessage(hWndMain, WM_COMMAND, IDM_HELP, 0);
	else if (StrEqual(cBut.get_attribute("id"), L"link_exit")) SendMessage(hWndMain, WM_SYSCOMMAND, SC_CLOSE, 0);
	else if (StrEqual(cBut.get_attribute("id"), L"link_more"))
	{
		if (fullWindow)
		{
			fullWindow = false;
			RECT rc; GetWindowRect(hWndMain, &rc);
			MoveWindow(hWndMain, rc.left, rc.top, rc.right - rc.left, 210, TRUE);

			exten_area.set_attribute("style", L"display: none;");
			link_more.set_text(L"▼ 高级模式");
		}
		else
		{
			fullWindow = true;
			RECT rc; GetWindowRect(hWndMain, &rc);
			MoveWindow(hWndMain, rc.left, rc.top, rc.right - rc.left, 450, TRUE);

			exten_area.set_attribute("style", L"");
			link_more.set_text(L"▲ 隐藏面板");
		}
	}
	else if (StrEqual(cBut.get_attribute("id"), L"link_kill")) {
		if (TForceKill()) {
			SetCtlStatus(false, (LPWSTR)L"已成功结束极域进程");
			XOutPutStatus(L"已成功结束极域进程");
		}else XOutPutStatus(L"无法结束极域进程 %s", TGetLastError());
	}
	else if (StrEqual(cBut.get_attribute("id"), L"link_rerun"))
	{
		LPWSTR jiyuPath = TGetLastJiYuPayh();
		if (StrEqual(jiyuPath, L"") || !PathFileExists(jiyuPath)) {
			XOutPutStatus(L"无法定位极域主进程位置 %s", jiyuPath);
			MessageBox(hWndMain, L"无法定位极域主进程位置，请手动启动。", L"错误", 0);
		}
		else {
			ShellExecute(hWndMain, L"open", jiyuPath, NULL, NULL, SW_SHOW);
			XOutPutStatus(L"已经启动极域 %s", jiyuPath);
		}
	}
	else if (StrEqual(cBut.get_attribute("id"), L"link_runcmd"))
	{
		json::value v = htmlayout::get_value(input_cmd);
		if (v.is_string()) 
		{
			aux::wchars str = v.get_chars();
			size_t len = str.length + 1;
			LPWSTR cmdsx = (LPWSTR)malloc(sizeof(WCHAR) *len);
			wcscpy_s(cmdsx, len, str.start);

			wstring cmdx(cmdsx);
			if (cmdx == L"") 
				MessageBox(hWndMain, L"请输入命令！", L"命令帮助", MB_ICONEXCLAMATION);		
			else {
				vector<wstring> cmds;
				SplitString(cmdx, cmds, L" ");
				OnRunCmd(&cmds, cmds.size());
			}
		}
	}
	else if (StrEqual(cBut.get_attribute("id"), L"link_cmds"))
	{
		MessageBox(hWndMain, L"killst \nrerunst \nss \nsss \nssss \n", L"命令帮助", 0);
	}
}
LRESULT OnWmTimer(HWND hWnd, WPARAM timerId)
{
	switch (timerId)
	{
	case TIMER_RESET_PID: ResetPid(); break;
	case TIMER_TOP_CHECK: {
		SetCkStatus(TRunCK(), WGetCkStatText());
		SetTimer(hWnd, TIMER_CK_DEALY_HIDE, 2000, NULL);
		break;
	}
	case TIMER_AOP: {
		SetWindowPos(hWndMain, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
		break;
	}
	case TIMER_CK_DEALY_HIDE: {
		KillTimer(hWnd, TIMER_CK_DEALY_HIDE);
		SetCkStatus(0, (LPWSTR)L"");
		break;
	}
	default:break;
	}
	return 0;
}
void OnHandleMsg(LPWSTR buff) {
	XOutPutStatus(L"Receive message : %s", buff);
	wstring act(buff);
	vector<wstring> arr;
	SplitString(act, arr, L":");
	if (arr.size() >= 2) {
		if (arr[0] == L"hkb") 
		{
			if (arr[1] == L"succ")
				SetCtlStatus(true, TGetCurrStatText());
			else if (arr[1] == L"immck")
				SendMessage(hWndMain, WM_TIMER, TIMER_TOP_CHECK, NULL);	
		}
		else if (arr[0] == L"wcd") {
			//wwcd
		}
	}
}
void OnWmUser(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
	if (lParam == WM_LBUTTONDBLCLK)
		SendMessage(hDlg, WM_COMMAND, IDM_SHOWMAIN, lParam);
	if (lParam == WM_RBUTTONDOWN)
	{
		POINT pt;
		GetCursorPos(&pt);//取鼠标坐标  
		SetForegroundWindow(hDlg);//解决在菜单外单击左键菜单不消失的问题  
		TrackPopupMenu(hMenuTray, TPM_RIGHTBUTTON, pt.x - 177, pt.y, NULL, hDlg, NULL);//显示菜单并获取选项ID  
	}
}

void SetCK(bool enable) 
{
	if (enable)
	{
		SetTimer(hWndMain, TIMER_TOP_CHECK, setCKinterval, NULL);
		XOutPutStatus(L"CK 正在运行");
		SendMessage(hWndMain, WM_TIMER, TIMER_TOP_CHECK, NULL);
	}
	else 
	{
		KillTimer(hWndMain, TIMER_TOP_CHECK);
		SetCkStatus(-1, (LPWSTR)L"CK 已停止");
		XOutPutStatus(L"CK 已停止");
	}
}
void ResetPid()
{
	if (TReset())
	{
		bool b = TLocated();
		SetProcStatus(b, b ? TGetCurrProcStatText() : (LPWSTR)L"");
		TSendCtlStat();
	}
}

void SetProcStatus(bool ctled, LPWSTR extendInfo)
{
	if (ctled) {
		stat_ico_proc_red.set_attribute("style", L"display: none;");
		stat_ico_proc_green.set_attribute("style", L"");
	}
	else {
		stat_ico_proc_green.set_attribute("style", L"display: none;");
		stat_ico_proc_red.set_attribute("style", L"");
		SetCtlStatus(false, NULL);
	}
	if (extendInfo != NULL) stat_proc_text.set_text(extendInfo);
}
void SetCtlStatus(bool ctled, LPWSTR extendInfo) {
	if (controlled != ctled) {
		if (ctled) {
			stat_ico_ctl_no.set_attribute("style", L"display: none;");
			stat_ico_ctl_yes.set_attribute("style", L"");

			stat_ctl_text.set_text(L"当前已控制极域主进程");
			stat_ctl_text.set_attribute("class", L"text-green");
		}
		else {
			stat_ico_ctl_yes.set_attribute("style", L"display: none;");
			stat_ico_ctl_no.set_attribute("style", L"");

			stat_ctl_text.set_attribute("class", L"text-red");
			stat_ctl_text.set_text(L"当前未控制极域主进程");
		}
		controlled = ctled;
	}
	if (extendInfo != NULL) stat_ctl_text2.set_text(extendInfo);
}
void SetCkStatus(int s, LPWSTR str)
{
	if (s == -1) {
		stat_ico_ck_grey.set_attribute("style", L"display:none;");
		stat_ico_ck_red.set_attribute("style",L"");
		stat_ico_ck_green.set_attribute("style", L"display:none;");

	}
	else if (s == 0) {
		stat_ico_ck_red.set_attribute("style", L"display:none;");
		stat_ico_ck_grey.set_attribute("style", L"");
		stat_ico_ck_green.set_attribute("style", L"display:none;");
	}
	else if (s == 1) {
		stat_ico_ck_red.set_attribute("style", L"display:none;");
		stat_ico_ck_green.set_attribute("style", L"");
		stat_ico_ck_grey.set_attribute("style", L"display:none;");
	}
	if(str!=NULL) stat_ck_text.set_text(str);
}
BOOL LoadMainHtml(HWND hWnd)
{
	BOOL result = FALSE;
	HRSRC hResource = FindResource(hInst, MAKEINTRESOURCE(IDR_HTML_MAIN), RT_HTML);
	if (hResource) {
		HGLOBAL hg = LoadResource(hInst, hResource);
		if (hg) {
			LPVOID pData = LockResource(hg);
			if (pData)
				result = HTMLayoutLoadHtml(hWnd, (LPCBYTE)pData, SizeofResource(hInst, hResource));
		}
	}
	hResource = FindResource(hInst, MAKEINTRESOURCE(IDR_CSS_MAIN), L"CSS");
	if (hResource) {
		HGLOBAL hg = LoadResource(hInst, hResource);
		if (hg) {
			LPVOID pData = LockResource(hg);
			if (pData)
				result = HTMLayoutSetCSS(hWnd, (LPCBYTE)pData, SizeofResource(hInst, hResource), L"", L"text/css");
		}
	}
	return result;
}
void CreateTrayIcon(HWND hDlg) {
	nid.cbSize = sizeof(nid);
	nid.hWnd = hDlg;
	nid.uID = 0;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_USER;
	nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_JIYUKILLER));
	lstrcpy(nid.szTip, L"JY Killer");
	Shell_NotifyIcon(NIM_ADD, &nid);
}

struct MainWindowDOMEventsHandlerType : htmlayout::event_handler
{
	MainWindowDOMEventsHandlerType() : event_handler(0xFFFFFFFF) {}
	virtual BOOL handle_event(HELEMENT he, BEHAVIOR_EVENT_PARAMS& params)
	{
		switch (params.cmd)
		{
		case BUTTON_CLICK: OnButtonClick(params.heTarget); break;// click on button
		case BUTTON_PRESS: break;// mouse down or key down in button
		case BUTTON_STATE_CHANGED:      break;
		case EDIT_VALUE_CHANGING:       break;// before text change
		case EDIT_VALUE_CHANGED:        break;//after text change
		case SELECT_SELECTION_CHANGED:  break;// selection in <select> changed
		case SELECT_STATE_CHANGED:      break;// node in select expanded/collapsed, heTarget is the node
		case POPUP_REQUEST: break;// request to show popup just received, 
				  //     here DOM of popup element can be modifed.
		case POPUP_READY:               break;// popup element has been measured and ready to be shown on screen,
											  //     here you can use functions like ScrollToView.
		case POPUP_DISMISSED:           break;// popup element is closed,
											  //     here DOM of popup element can be modifed again - e.g. some items can be removed
											  //     to free memory.
		case MENU_ITEM_ACTIVE:                // menu item activated by mouse hover or by keyboard
			break;
		case MENU_ITEM_CLICK:                 // menu item click 
			break;
			// "grey" event codes  - notfications from behaviors from this SDK 
		case HYPERLINK_CLICK: OnLinkClick(params.heTarget);  break;// hyperlink click
		case TABLE_HEADER_CLICK:        break;// click on some cell in table header, 
											  //     target = the cell, 
											  //     reason = index of the cell (column number, 0..n)
		case TABLE_ROW_CLICK:           break;// click on data row in the table, target is the row
											  //     target = the row, 
											  //     reason = index of the row (fixed_rows..n)
		case TABLE_ROW_DBL_CLICK:       break;// mouse dbl click on data row in the table, target is the row
											  //     target = the row, 
											  //     reason = index of the row (fixed_rows..n)

		case ELEMENT_COLLAPSED:         break;// element was collapsed, so far only behavior:tabs is sending these two to the panels
		case ELEMENT_EXPANDED:          break;// element was expanded,
		case DOCUMENT_COMPLETE: OnDocumentComplete(); break;
		}
		return FALSE;
	}

} MainWindowDOMEventsHandlerType;

LRESULT CALLBACK MainWindowHTMLayoutNotifyHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LPVOID vParam)
{
	// all HTMLayout notification are comming here.
	NMHDR*  phdr = (NMHDR*)lParam;

	switch (phdr->code)
	{
	case HLN_CREATE_CONTROL:    break; //return OnCreateControl((LPNMHL_CREATE_CONTROL) lParam);
	case HLN_CONTROL_CREATED:   break; //return OnControlCreated((LPNMHL_CREATE_CONTROL) lParam);
	case HLN_DESTROY_CONTROL:   break; //return OnDestroyControl((LPNMHL_DESTROY_CONTROL) lParam);
	case HLN_LOAD_DATA:         break;
	case HLN_DATA_LOADED:       break; //return OnDataLoaded((LPNMHL_DATA_LOADED)lParam);
	case HLN_DOCUMENT_COMPLETE: break; //return OnDocumentComplete();
	case HLN_ATTACH_BEHAVIOR:   break;
	}
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 0;
	BOOL bHandled;

	lResult = HTMLayoutProcND(hWnd, message, wParam, lParam, &bHandled);
	if (!bHandled)
	{
		switch (message)
		{
		case WM_CREATE: { 
			hWndMain = hWnd;

			HTMLayoutSetCallback(hWnd, &MainWindowHTMLayoutNotifyHandler, 0);
			// attach DOM events handler so we will be able to receive DOM events like BUTTON_CLICK, HYPERLINK_CLICK, etc.
			htmlayout::attach_event_handler(hWnd, &MainWindowDOMEventsHandlerType);

			if (!LoadMainHtml(hWnd)) return FALSE;  

			OnDocumentComplete();

			return OnWmCreate(hWnd); 
		}
		case WM_COMMAND: OnWmCommand(hWnd, wParam); break;
		case WM_SYSCOMMAND: {
			switch (wParam)
			{
			case SC_MINIMIZE: ShowWindow(hWnd, SW_HIDE);  break;
			case SC_CLOSE: if (!controlled || MessageBox(hWnd, L"极域正在运行，退出本软件将导致极域脱离控制，您是否还要退出？", L"注意", MB_YESNO | MB_ICONEXCLAMATION) == IDYES) { isUserCancel = true;  XQuitApp(); } break;
			default: return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		}
		case WM_COPYDATA: {
			PCOPYDATASTRUCT  pCopyDataStruct = (PCOPYDATASTRUCT)lParam;
			if (pCopyDataStruct->cbData > 0)
			{
				WCHAR recvData[256] = { 0 };
				wcsncpy_s(recvData, (WCHAR *)pCopyDataStruct->lpData, pCopyDataStruct->cbData);
				OnHandleMsg(recvData);
			}
			break;
		}
		case WM_SHOWWINDOW: {
			if (wParam)
			{
				if (firstShow) 
				{
					//窗口居中
					RECT rect; GetWindowRect(hWnd, &rect);
					rect.left = (screenWidth - (rect.right - rect.left)) / 2;
					rect.top = (screenHeight - (rect.bottom - rect.top)) / 2 - 60;
					SetWindowPos(hWnd, 0, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

					firstShow = false;
				}
			}
			break;
		}
		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;
		}
		case WM_DESTROY:  OnWmDestroy(hWnd); break;
		case WM_TIMER: return OnWmTimer(hWnd, wParam);
		case WM_USER: OnWmUser(hWnd, wParam, lParam); break;
		default: return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	return lResult;
}