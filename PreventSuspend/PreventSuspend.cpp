//----------------------------------------------------------------
#include "stdafx.h"
#include "PreventSuspend.h"
//----------------------------------------------------------------
#define MY_NOTIFYICON  (WM_APP + 108)
#define APP_NAME       (L"PreventSuspend")
static HINSTANCE g_hInst = NULL;
static NOTIFYICONDATA g_nIcon = {0};
//--------------------------------------------------------------------
static
void
ShowTrayMenu(
	HWND hWnd
)
{
	HMENU hMenu;
	POINT MenuPos;

	hMenu = CreatePopupMenu();
	AppendMenu(hMenu, MF_STRING, IDM_ABOUT, L"&About...");
	AppendMenu(hMenu, MF_SEPARATOR, 0, 0);
	AppendMenu(hMenu, MF_STRING, IDM_EXIT, L"E&xit");
	SetForegroundWindow(hWnd);
	GetCursorPos(&MenuPos);
	TrackPopupMenu(hMenu, TPM_RIGHTALIGN,
		MenuPos.x, MenuPos.y, 0, hWnd, NULL);
	DestroyMenu(hMenu);
}
//----------------------------------------------------------------
static
BOOL
OnCreate(
	HWND hWnd
)
{
	HANDLE hThread = NULL;
	WCHAR cFileName[MAX_PATH] = {0};

	GetModuleFileName(g_hInst, cFileName, MAX_PATH);
	g_nIcon.cbSize = sizeof(NOTIFYICONDATA);
	g_nIcon.uID = 1;
	g_nIcon.hWnd = hWnd;
	g_nIcon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	ExtractIconEx(cFileName, 0, NULL, &(g_nIcon.hIcon), 1);	// Small.ico
	g_nIcon.uCallbackMessage = MY_NOTIFYICON;
	lstrcpy(g_nIcon.szTip, APP_NAME);
	Shell_NotifyIcon(NIM_ADD, &g_nIcon);		// トレイ登録

	// 見えないところへ……
	SetWindowPos(hWnd, HWND_BOTTOM, 30000, 30000, 0, 0, SWP_NOSIZE);
	ShowWindow(hWnd, SW_HIDE);

	// タスクバーから消す
	LONG exStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	SetWindowLong(hWnd, GWL_EXSTYLE, exStyle | WS_EX_TOOLWINDOW);

	SetTimer(hWnd, 1234, 60 * 1000, NULL);

	return FALSE;
}
//----------------------------------------------------------------
static
BOOL
OnPowerBroadcast(
	HWND hWnd,
	WPARAM wParam,
	LPARAM lParam
)
{
	switch (wParam) {
	case PBT_APMQUERYSUSPEND:
		SetWindowLong(hWnd, DWL_MSGRESULT, BROADCAST_QUERY_DENY);
		return TRUE;
	default:
		break;
	}

	return FALSE;
}
//----------------------------------------------------------------
static
INT_PTR
CALLBACK
DlgProc(
	HWND hWnd,     // ダイアログボックスのハンドル
	UINT uMsg,     // メッセージ
	WPARAM wParam, // 最初のメッセージパラメータ
	LPARAM lParam  // 2 番目のメッセージパラメータ
)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		return OnCreate(hWnd);
	case WM_POWERBROADCAST:
		return OnPowerBroadcast(hWnd, wParam, lParam);
	case MY_NOTIFYICON:
		switch(lParam){
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			ShowTrayMenu(hWnd);		// メニュー
			return TRUE;
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDM_EXIT:
		case IDOK:
		case IDCANCEL:
			KillTimer(hWnd, 1234);
			EndDialog(hWnd, 0);
			break;
		case IDM_ABOUT:
			MessageBox(
				hWnd, L"PreventSuspend v1.0.0\n\nCopyright(c) 2019 tomarin",
				L"About", MB_OK | MB_ICONINFORMATION
			);
		}
		break;
	case WM_TIMER:
		mouse_event(0, 0, 0, 0, NULL);
		break;
	}

	return FALSE;
}
//----------------------------------------------------------------
int APIENTRY _tWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow
)
{
	g_hInst = hInstance;

	SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED);

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_MAIN), NULL, (DLGPROC)DlgProc);

	SetThreadExecutionState(ES_CONTINUOUS);

	return 0;
}
//----------------------------------------------------------------
