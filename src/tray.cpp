
#include "MyKeepAlive.h"

#include <CommCtrl.h>

using namespace std;

HWND hwndTray                   = nullptr;
NOTIFYICONDATA nid              = {};
const UINT WM_USER_SHELLICON    = WM_USER + 1;
const UINT WM_POKE              = WM_USER + 101;
const LPCWSTR szTitle           = L"keepalive";
const LPCWSTR szWindowClass     = L"keepalive_traywindow";
const UINT TimerMS              = 10000; // 10 seconds

#define IDM_EXIT                111
#define IDM_TOGGLEPAUSE         112
#define IDM_TOGGLEONSTARTUP     113
#define IDM_TOGGLESCHEDULE      114
#define IDM_CONFIGURE           115


// On the timer, if not paused, inject ghost input
void TimerCallback(HWND, UINT, UINT_PTR, DWORD)
{
    if (!IsPaused())
    {
        if (IsScheduled())
        {
            SYSTEMTIME time;
            GetLocalTime(&time);
            if (!TimeInSchedule(time)) {
                // bail, we are outside the schedule
                return;
            }
        }

        InjectGhostInput();
    }
}


//
// Tray window updates with shell to add itself into the taskbar tray
//

void UpdateShellIconInfo(DWORD msg)
{
    // Send updated notify icon info to shell
    if (!Shell_NotifyIcon(msg, &nid))
    {
        // This does sometimes fail, shows shell crashed....
        // todo, might need to cleanup/ relaunch??
        Warning(L"Shell_NotifyIcon failed!");
    }
}

void UpdateTrayWindow()
{
    static HICON hIconOn = LoadIcon(hInstance, (LPCTSTR)MAKEINTRESOURCE(IDI_HANDPIC_ACTIVE));
    static HICON hIconOff = LoadIcon(hInstance, (LPCTSTR)MAKEINTRESOURCE(IDI_HANDPIC));

    // Update the icon to reflect pause state
    nid.hIcon = (IsPaused() ? hIconOff : hIconOn);

    // Update the tooltip text to reflect pause state
    wcscpy_s(nid.szTip, IsPaused() ?
        L"Keep Alive - Paused" :
        L"Keep Alive - Running");

    UpdateShellIconInfo(NIM_MODIFY);
}

void InitShellIconInfo()
{
    // Initialize the icon data and add to taskbar (NIM_ADD)
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwndTray;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_SHOWTIP | NIF_TIP;
    nid.uCallbackMessage = WM_USER_SHELLICON;
    nid.uID = 13542;
    UpdateShellIconInfo(NIM_ADD);

    // Update the icon and tooltip text for initial paused state
    UpdateTrayWindow();
}


//
// On right click, show a context menu with exit, pause, on startup
//

void RightClickMenu()
{
    POINT pt;
    GetCursorPos(&pt);
    pt = KeepPointInRect(pt, WorkAreaFromPoint(pt));

    HMENU hMenu = CreatePopupMenu();

    // schedule menu items
    InsertMenu(hMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_CONFIGURE, L"Configure...");
    InsertMenu(hMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_TOGGLESCHEDULE, L"Use Schedule");
    CheckMenuItem(hMenu, IDM_TOGGLESCHEDULE, IsScheduled() ? MF_CHECKED : MF_UNCHECKED);

    // pause
    InsertMenu(hMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_TOGGLEPAUSE, L"Pause");
    CheckMenuItem(hMenu, IDM_TOGGLEPAUSE, IsPaused() ? MF_CHECKED : MF_UNCHECKED);

    // run on startup
    InsertMenu(hMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_TOGGLEONSTARTUP, L"Run On Startup");
    CheckMenuItem(hMenu, IDM_TOGGLEONSTARTUP, IsRunOnStartupSet() ? MF_CHECKED : MF_UNCHECKED);

    // exit
    InsertMenu(hMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_EXIT, L"Exit");

    WellBehavedTrackPopup(hwndTray, hMenu, pt);
}


//
// WndProc
//

LRESULT CALLBACK TrayWindowWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    // Initialize timer and shell icon info
    case WM_CREATE:
        hwndTray = hwnd;
        SetTimer(hwnd, 101, TimerMS, (TIMERPROC)TimerCallback);
        InitShellIconInfo();
        break;

    // On startup, finds existing window and pokes. Return non zero.
    case WM_POKE:
        return 1;

    // Input messages
    case WM_USER_SHELLICON:
        switch (LOWORD(lParam))
        {
        case WM_RBUTTONDOWN:
            RightClickMenu();
            break;

        // Double click toggles pause and hides preview window
        case WM_LBUTTONDBLCLK:
            ToggleIsPaused();
            ShowHidePreviewWindow(false);
            break;

        // Clicks shows the preview window
        case WM_LBUTTONDOWN:
            ShowHidePreviewWindow(true);
            break;
        }
        break;

    // Right click menu messages
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_EXIT:
            UpdateShellIconInfo(NIM_DELETE);
            PostQuitMessage(0);
            break;

        case IDM_TOGGLEPAUSE:
            ToggleIsPaused();
            break;

        case IDM_TOGGLEONSTARTUP:
            ToggleRunOnStartup();
            break;

        case IDM_TOGGLESCHEDULE:
            ToggleSchedule();
            break;

        case IDM_CONFIGURE:
            Configure();
            break;
        }
        break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}


//
// Initialize
//

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
