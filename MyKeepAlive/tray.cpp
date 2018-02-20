
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

const UINT TimerMS = 10000;          // 10 seconds
const UINT LongTimerMS = 60000;      // 1 minute
const UINT DelayTimeoutM = 60 * 5;   // 5 hours

NOTIFYICONDATA nid = {};
HWND hwndTray = nullptr;
int DelayRemainingM = -1;
int TotalTimeRunningM = 0;

void UpdateTrayIcon()
{
    // Called when paused changes, update tray icon

    nid.hIcon = LoadIcon(hInstance,
        (LPCTSTR)MAKEINTRESOURCE(paused ? IDI_HANDPIC : IDI_HANDPIC_ACTIVE));

    if (!Shell_NotifyIcon(NIM_MODIFY, &nid))
    {
        Error(L"Shell_NotifyIcon failed!");
    }
}

void UpdateTooltipText()
{
    // Called when active/ delay state changes, update tooltip (hover bubble) text

    if (paused)
    {
        wcscpy_s(nid.szTip, L"Keep Alive - Paused");
    }
    else if (DelayRemainingM >= 0)
    {
        UINT days, hours, minutes;
        DaysMinsSecsFromMinutes(DelayRemainingM, &days, &hours, &minutes);

        swprintf(nid.szTip, 128,
                 L"Keep Alive - Timer Running\n%i hr %i min remaining",
                 hours, minutes);
    }
    else
    {
        wcscpy_s(nid.szTip, L"Keep Alive - Running");
    }

    if (!Shell_NotifyIcon(NIM_MODIFY, &nid))
    {
        Error(L"Setting tooltip text failed! - Shell_NotifyIcon(MODIFY)");
    }
}

void Click(bool rightClick)
{
    if (rightClick)
    {
        // Right click creates an on-the-fly menu and shows it near the cursor

        HMENU hMenu = CreatePopupMenu();

        // Add the exit option
        InsertMenu(hMenu, 0xFFFFFFFF,
            MF_BYPOSITION | MF_STRING,
            IDM_EXIT, L"Exit");

        // Add the pause option and 'check' it if we're paused
        InsertMenu(hMenu, 0xFFFFFFFF,
            MF_BYPOSITION | MF_STRING,
            IDM_TOGGLEPAUSE, L"Pause");

        CheckMenuItem(hMenu, IDM_TOGGLEPAUSE,
            paused ? MF_CHECKED : MF_UNCHECKED);

        if (!paused)
        {
            // Add the 'delay pause' button (if not paused), and check if there's a delay

            bool delayset = (DelayRemainingM >= 0);

            WCHAR mnItemBuf[100];
            if (delayset)
            {
                UINT days, hours, minutes;
                DaysMinsSecsFromMinutes(DelayRemainingM, &days, &hours, &minutes);

                swprintf(BUFSTR(mnItemBuf), 100,
                    L"Pause in %i hr %i min", hours, minutes);
            }
            else
            {
                swprintf(BUFSTR(mnItemBuf), 100, L"Pause in 5 hours");
            }

            InsertMenu(hMenu, 0xFFFFFFFF,
                MF_BYPOSITION | MF_STRING,
                IDM_TOGGLE5HRDELAY, BUFSTR(mnItemBuf));

            CheckMenuItem(hMenu, IDM_TOGGLE5HRDELAY,
                delayset ? MF_CHECKED : MF_UNCHECKED);
        }

        // Open the menu at the cursor pos, but within the work area
        POINT pt;
        GetCursorPos(&pt);
        POINT ptMenu = KeepPointInRect(pt, WorkAreaFromPoint(pt));
        WellBehavedTrackPopup(hwndTray, hMenu, ptMenu);
    }
    else
    {
        // Left click shows the preview window
        ShowPreviewWindow();
    }
}

void Toggle5HrDelay()
{
    if (DelayRemainingM < 0)
    {
        DelayRemainingM = DelayTimeoutM;
    }
    else
    {
        DelayRemainingM = -1;
    }

    UpdateTooltipText();
}

void TogglePaused()
{
    paused = !paused;

    UpdateTrayIcon();
    UpdateTooltipText();
}

void Tick(bool LongTimer)
{
    if (LongTimer) /* IDT_TIMER_LONG, fires every minute */
    {
        TotalTimeRunningM++;

        if (DelayRemainingM == 0)
        {
            paused = true;
        }
        else if (DelayRemainingM > 0)
        {
            DelayRemainingM--;
        }
    }
    else /* IDT_TIMER, fires every ten seconds */
    {
        if (!paused)
        {
            // Inject bogus keyboard input
            static INPUT i = { INPUT_KEYBOARD,{ VK_F24, 0, 0, 0, 0 } };
            SendInput(1, &i, sizeof(INPUT));
        }
    }

    UpdateTrayIcon();
    UpdateTooltipText();
}

bool InitTrayWindow(HWND hwnd)
{
    hwndTray = hwnd;
    SetTimer(hwnd, IDT_TIMER, TimerMS, nullptr);
    SetTimer(hwnd, IDT_TIMER_LONG, LongTimerMS, nullptr);

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_SHOWTIP | NIF_TIP;
    nid.uCallbackMessage = WM_USER_SHELLICON;
    nid.uID = 13542; // each tray icon declares a unique id? weird.

    if (!Shell_NotifyIcon(NIM_ADD, &nid))
    {
        Error(L"Shell_NotifyIcon failed!");
        return false;
    }

    UpdateTooltipText();
    UpdateTrayIcon();
    return true;
}

void OnExit()
{
    // Tray closing, deregister, destroy preview window, exit message loop.

    Shell_NotifyIcon(NIM_DELETE, &nid);
    DestroyWindow(hwndPreview);
    PostQuitMessage(0);
}

LRESULT CALLBACK TrayWindowWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        if (!InitTrayWindow(hwnd))
        {
            return -1;
        }
        break;

    case WM_TIMER:
        Tick(LOWORD(wParam) == IDT_TIMER_LONG);
        break;

    case WM_USER_SHELLICON:
        switch (LOWORD(lParam))
        {
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
            Click(LOWORD(lParam) == WM_RBUTTONDOWN);
            break;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_EXIT:
            OnExit();
            break;

        case IDM_TOGGLEPAUSE:
            TogglePaused();
            break;

        case IDM_TOGGLE5HRDELAY:
            Toggle5HrDelay();
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

