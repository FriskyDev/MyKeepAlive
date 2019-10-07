
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

NOTIFYICONDATA nid = {};

void UpdateShellIconInfo(DWORD msg)
{
    if (!Shell_NotifyIcon(msg, &nid))
    {
        //Error(L"Shell_NotifyIcon failed!");
    }
}

void UpdateRunningState()
{
    static HICON hIconOn = LoadIcon(hInstance,
        (LPCTSTR)MAKEINTRESOURCE(IDI_HANDPIC_ACTIVE));
    static HICON hIconOff = LoadIcon(hInstance,
        (LPCTSTR)MAKEINTRESOURCE(IDI_HANDPIC));

    nid.hIcon = (Paused() ? hIconOff : hIconOn);
    wcscpy_s(nid.szTip, Paused() ? L"Keep Alive - Paused" : L"Keep Alive - Running");

    UpdateShellIconInfo(NIM_MODIFY);
}

void RightClickMenu(HWND hwnd, POINT pt)
{
    HMENU hMenu = CreatePopupMenu();

    InsertMenu(hMenu, 0xFFFFFFFF,
        MF_BYPOSITION | MF_STRING,
        IDM_EXIT, L"Exit");

    InsertMenu(hMenu, 0xFFFFFFFF,
        MF_BYPOSITION | MF_STRING,
        IDM_TOGGLEPAUSE, L"Pause");

    CheckMenuItem(hMenu, IDM_TOGGLEPAUSE,
        Paused() ? MF_CHECKED : MF_UNCHECKED);

    WellBehavedTrackPopup(hwnd, hMenu, pt);
}

LRESULT CALLBACK TrayWindowWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateTimers(hwnd);

        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwnd;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_SHOWTIP | NIF_TIP;
        nid.uCallbackMessage = WM_USER_SHELLICON;
        nid.uID = 13542;
        UpdateShellIconInfo(NIM_ADD);

        UpdateRunningState();
        break;

    case WM_USER_SHELLICON:
		switch (LOWORD(lParam))
		{
		case WM_RBUTTONDOWN:
		{
			POINT pt;
			GetCursorPos(&pt);
			pt = KeepPointInRect(pt, WorkAreaFromPoint(pt));
			RightClickMenu(hwnd, pt);
			break;
		}
		case WM_LBUTTONDOWN:
			ShowHidePreview(true);
			break;

		case WM_LBUTTONDBLCLK:
			TogglePaused();
			ShowHidePreview(false);
			break;
		}
		break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_EXIT:
            UpdateShellIconInfo(NIM_DELETE);
            PostQuitMessage(0);
            break;

        case IDM_TOGGLEPAUSE:
            TogglePaused();
            break;
        }
        break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

bool CreateTrayWindow()
{
    const LPCWSTR szTitle = L"keepalive";
    const LPCWSTR szWindowClass = L"keepalive_class";

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
        Error(L"Register Tray failed!");
        return false;
    }

    if (CreateWindow(szWindowClass,
                     szTitle,
                     WS_OVERLAPPEDWINDOW,
                     0, 0, 0, 0,
                     nullptr, nullptr, hInstance, nullptr) == nullptr)
    {
        Error(L"Create Tray failed!");
        return false;
    }

    return true;
}

