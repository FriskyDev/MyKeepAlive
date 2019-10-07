
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

#define IDM_EXIT            111
#define IDM_TOGGLEPAUSE     112
#define	WM_USER_SHELLICON   WM_USER + 1
const LPCWSTR szTitle = L"keepalive";
const LPCWSTR szWindowClass = L"keepalive_traywindow";
const UINT WM_POKE = WM_USER + 101;

void UpdateState();

//
// Global paused, starts false (aka not paused)
//
bool paused = false;

bool Paused()
{
	return paused;
}

void TogglePaused()
{
	paused = !paused;

	UpdateState();
}


//
// Every 10 seconds, inject a zero-zero mouse input (unless paused)
//
const UINT TimerMS = 10000; // 10 seconds

void TimerCallback(HWND, UINT, UINT_PTR, DWORD)
{
	if (!Paused())
	{
		static INPUT i = { INPUT_MOUSE,{} };
		SendInput(1, &i, sizeof(INPUT));
	}
}

//
// Updates to the tray icon are given to shell using Shell_NotifyIcon
//
NOTIFYICONDATA nid = {};

void UpdateShellIconInfo(DWORD msg)
{
    if (!Shell_NotifyIcon(msg, &nid))
    {
		// This does sometimes fail, shows shell crashed....
		// todo, might need to cleanup/ relaunch??
		Warning(L"Shell_NotifyIcon failed!");
    }
}

void InitShellIconInfo(HWND hwnd)
{
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hwnd;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_SHOWTIP | NIF_TIP;
	nid.uCallbackMessage = WM_USER_SHELLICON;
	nid.uID = 13542;
	UpdateShellIconInfo(NIM_ADD);

	UpdateState();
}


//
// On state changes, update tray icon, tooltip text, etc
//
void UpdateState()
{
    static HICON hIconOn = LoadIcon(hInstance,
        (LPCTSTR)MAKEINTRESOURCE(IDI_HANDPIC_ACTIVE));
    static HICON hIconOff = LoadIcon(hInstance,
        (LPCTSTR)MAKEINTRESOURCE(IDI_HANDPIC));

    nid.hIcon = (Paused() ? hIconOff : hIconOn);
    wcscpy_s(nid.szTip, Paused() ? L"Keep Alive - Paused" : L"Keep Alive - Running");

    UpdateShellIconInfo(NIM_MODIFY);
}


//
// On right click, build a context menu with pause and exit options
//
void RightClickMenu(HWND hwnd)
{
	POINT pt;
	GetCursorPos(&pt);
	pt = KeepPointInRect(pt, WorkAreaFromPoint(pt));

    HMENU hMenu = CreatePopupMenu();

    InsertMenu(hMenu, 0xFFFFFFFF,
        MF_BYPOSITION | MF_STRING,
        IDM_TOGGLEPAUSE, L"Pause");

    CheckMenuItem(hMenu, IDM_TOGGLEPAUSE,
        Paused() ? MF_CHECKED : MF_UNCHECKED);

	// keep exit as last, closest to cursor when opened
	InsertMenu(hMenu, 0xFFFFFFFF,
		MF_BYPOSITION | MF_STRING,
		IDM_EXIT, L"Exit");

    WellBehavedTrackPopup(hwnd, hMenu, pt);
}

void OnContextMenuSelection(WORD cmd)
{
	switch (cmd)
	{
	case IDM_EXIT:
		UpdateShellIconInfo(NIM_DELETE);
		PostQuitMessage(0);
		break;

	case IDM_TOGGLEPAUSE:
		TogglePaused();
		break;
	}

}

//
// Input handlers (input over tray icon)
//
void OnInput(HWND hwnd, WORD cmd)
{
	switch (cmd)
	{
	// Right button, show context menu
	case WM_RBUTTONDOWN:
		RightClickMenu(hwnd);
		break;

	// Left button, show preview window
	case WM_LBUTTONDOWN:
		ShowHidePreview(true);
		break;

	// double click, toggle pause
	case WM_LBUTTONDBLCLK:
		TogglePaused();
		ShowHidePreview(false);
		break;
	}
}

LRESULT CALLBACK TrayWindowWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
		// Setup timer, and initialize icon state
		SetTimer(hwnd, 101, TimerMS, (TIMERPROC)TimerCallback);
		InitShellIconInfo(hwnd);
        break;

	// Input messages are sent via our callback message
    case WM_USER_SHELLICON:
		OnInput(hwnd, LOWORD(lParam));
		break;

	// Right click messages are sent within WM_COMMAND
	case WM_COMMAND:
		OnContextMenuSelection(LOWORD(wParam));
        break;

	// On startup, finds existing window and pokes. Return non zero.
	case WM_POKE: return 1;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

bool IsAlreadyRunning()
{
	HWND hwnd = FindWindow(szWindowClass, szTitle);
	DWORD_PTR dw = 0;
	return ((hwnd) &&
		(SendMessageTimeout(hwnd, WM_POKE, 0, 0, 0, 100, &dw) != 0) &&
		(dw != 0));
}

bool CreateTrayWindow()
{
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = TrayWindowWndProc;
    wcex.hInstance = hInstance;
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = nullptr;
    if (RegisterClassEx(&wcex) == 0)
    {
        Error(L"RegisterWindow failed on tray window!");
        return false;
    }

	HWND hwnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		0, 0, 0, 0,
		nullptr, nullptr, hInstance, nullptr);

	if (!hwnd)
    {
        Error(L"CreateWindow failed on tray window!");
        return false;
    }

    return true;
}

