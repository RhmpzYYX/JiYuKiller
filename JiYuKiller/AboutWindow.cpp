#include "stdafx.h"
#include "AboutWindow.h"
#include "htmlayout.h"
#include "resource.h"
#include "StringHlp.h"
#include "MainWindow.h"

extern int screenWidth;
extern int screenHeight;

extern HINSTANCE hInst;

bool aboutFirstShow = true;
HWND hWndAbout = NULL;

ATOM ARegisterClass() {

	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = AboutWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInst;
	wcex.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_JIYUKILLER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"XAbout";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	if (RegisterClassExW(&wcex) || GetLastError() == ERROR_CLASS_ALREADY_EXISTS)
		return TRUE;
	return FALSE;
}
BOOL ARunAboutWindow(HWND hWndParent) 
{
	aboutFirstShow = true;

	if (ARegisterClass())
	{

		hWndAbout = CreateWindowW(L"XAbout", L"关于 JY Killer", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
			CW_USEDEFAULT, 0, 400, 410, hWndParent, nullptr, hInst, nullptr);

		if (!hWndAbout) return FALSE;

		EnableWindow(hWndParent, FALSE);

		ShowWindow(hWndAbout, SW_SHOW);
		UpdateWindow(hWndAbout);

		MSG msg;
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		EnableWindow(hWndParent, TRUE);
		SetForegroundWindow(hWndParent);

	}

	return TRUE;
}


void AOnLinkClick(HELEMENT link)
{
	htmlayout::dom::element cBut = link;
	 if (StrEqual(cBut.get_attribute("id"), L"link_exit")) SendMessage(hWndAbout, WM_SYSCOMMAND, SC_CLOSE, 0);
	 if (StrEqual(cBut.get_attribute("id"), L"link_github"))ShellExecute(hWndAbout, L"open", L"https://github.com/717021/JiYuKiller", NULL, NULL, SW_SHOW);
	 if (StrEqual(cBut.get_attribute("id"), L"link_help"))ShellExecute(hWndAbout, L"open", L"https://github.com/717021/JiYuKiller/blob/master/README.md", NULL, NULL, SW_SHOW);
}

struct AboutWindowDOMEventsHandlerType : htmlayout::event_handler
{
	AboutWindowDOMEventsHandlerType() : event_handler(0xFFFFFFFF) {}
	virtual BOOL handle_event(HELEMENT he, BEHAVIOR_EVENT_PARAMS& params)
	{
		switch (params.cmd)
		{
		case BUTTON_CLICK: break;// click on button
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
		case HYPERLINK_CLICK: AOnLinkClick(params.heTarget);  break;// hyperlink click
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
		case DOCUMENT_COMPLETE: break;
		}
		return FALSE;
	}

} AboutWindowDOMEventsHandlerType;

LRESULT CALLBACK AboutWindowHTMLayoutNotifyHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LPVOID vParam)
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

BOOL ALoadMainHtml(HWND hWnd)
{
	BOOL result = FALSE;
	HRSRC hResource = FindResource(hInst, MAKEINTRESOURCE(IDR_HTML_ABOUT), RT_HTML);
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

LRESULT CALLBACK AboutWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 0;
	BOOL bHandled;

	lResult = HTMLayoutProcND(hWnd, message, wParam, lParam, &bHandled);
	if (!bHandled)
	{
		switch (message)
		{
		case WM_CREATE: {
			HTMLayoutSetCallback(hWnd, &AboutWindowHTMLayoutNotifyHandler, 0);
			// attach DOM events handler so we will be able to receive DOM events like BUTTON_CLICK, HYPERLINK_CLICK, etc.
			htmlayout::attach_event_handler(hWnd, &AboutWindowDOMEventsHandlerType);
			if (!ALoadMainHtml(hWnd)) return FALSE;
		}
		case WM_SYSCOMMAND: {
			switch (wParam)
			{
			case SC_CLOSE: DestroyWindow(hWnd); break;
			default: return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		}
		case WM_SHOWWINDOW: {
			if (wParam)
			{
				if (aboutFirstShow)
				{
					//窗口居中
					RECT rect; GetWindowRect(hWnd, &rect);
					rect.left = (screenWidth - (rect.right - rect.left)) / 2;
					rect.top = (screenHeight - (rect.bottom - rect.top)) / 2 - 60;
					SetWindowPos(hWnd, 0, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

					aboutFirstShow = false;
				}
			}
			break;
		}
		case WM_DESTROY:  PostQuitMessage(0); break;
		default: return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	return lResult;
}