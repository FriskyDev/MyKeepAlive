
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

NOTIFYICONDATA nid = {};
void UpdateShellIconInfo(DWORD msg)
{
    if (!Shell_NotifyIcon(msg, &nid))
    {
        Error(L"Shell_NotifyIcon failed!");
    }
}

void UpdateIconAndTooltip()
{
    const bool paused = Paused();
    const bool delayed = Delayed();
    UINT hours, minutes;
    HrsMinDelayed(&hours, &minutes);

    static HICON hIconOn = LoadIcon(hInstance,
        (LPCTSTR)MAKEINTRESOURCE(IDI_HANDPIC_ACTIVE));
    static HICON hIconOff = LoadIcon(hInstance,
        (LPCTSTR)MAKEINTRESOURCE(IDI_HANDPIC));

    static LPCWSTR strTooltipPause = L"Keep Alive - Paused";
    static LPCWSTR strTooltipRunning = L"Keep Alive - Paused";

    // Update the tray icon
    nid.hIcon = (paused ? hIconOff : hIconOn);

    // Update the tooltip text
    if (paused || !delayed)
    {
        wcscpy_s(nid.szTip, paused ? strTooltipPause : strTooltipRunning);
    }
    else
    {
        swprintf(nid.szTip, 128,
            L"Keep Alive - Timer Running\n%i hr %i min remaining",
            hours, minutes);
    }

    // Send updates to shell
    UpdateShellIconInfo(NIM_MODIFY);
}

LRESULT CALLBACK TrayWindowWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        // Register the timers
        CreateTimers(hwnd);

        // Register the Notify Icon
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwnd;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_SHOWTIP | NIF_TIP;
        nid.uCallbackMessage = WM_USER_SHELLICON;
        nid.uID = 13542; // not sure what this is...
        UpdateShellIconInfo(NIM_ADD);

        // Set initial icon and tooltip text
        UpdateIconAndTooltip();
        break;

    case WM_TIMER:
        Callback(LOWORD(wParam));
        break;

    case WM_USER_SHELLICON:
        switch (LOWORD(lParam))
        {
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
            if (LOWORD(lParam) == WM_RBUTTONDOWN)
            {
                POINT pt;
                GetCursorPos(&pt);
                pt = KeepPointInRect(pt, WorkAreaFromPoint(pt));
                RightClickMenu(hwnd, pt);
            }
            else
            {
                ShowHidePreview(true);
            }
            break;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_EXIT:
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;

        case IDM_TOGGLEPAUSE:
            TogglePaused();
            break;

        case IDM_TOGGLE5HRDELAY:
            ToggleDelay();
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

