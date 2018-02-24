
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

NOTIFYICONDATA nid = {};
HWND hwndTray = nullptr;
const UINT nidID = 13542;

void InjectBogusKeyboardInput()
{
    static INPUT i = { INPUT_KEYBOARD, { VK_F24, 0, 0, 0, 0 } };
    SendInput(1, &i, sizeof(INPUT));
}

void UpdateIconAndTooltip()
{
    static bool pauseLast = false;
    static bool delayLast = false;
    static int delayMLast = -1; // always update on first call

    const bool paused = gTimer->Paused();
    const bool delayed = gTimer->Delayed();
    UINT days, hours, minutes;
    gTimer->DaysHrsMinDelayed(&days, &hours, &minutes);

    // Return if there's nothing to do
    if ((paused == pauseLast) ||
        (delayed == delayLast) ||
        ((int)minutes == delayMLast))
    {
        return;
    }

    static HICON hIconOn = LoadIcon(hInstance,
        (LPCTSTR)MAKEINTRESOURCE(IDI_HANDPIC_ACTIVE));
    static HICON hIconOff = LoadIcon(hInstance,
        (LPCTSTR)MAKEINTRESOURCE(IDI_HANDPIC));

    // Update the tray icon
    nid.hIcon = (paused ? hIconOff : hIconOn);

    static LPCWSTR strTooltipPause = L"Keep Alive - Paused";
    static LPCWSTR strTooltipRunning = L"Keep Alive - Paused";

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
    if (!Shell_NotifyIcon(NIM_MODIFY, &nid))
    {
        Error(L"Shell_NotifyIcon failed!");
    }
}

void RightClickMenu()
{
    HMENU hMenu = CreatePopupMenu();

    InsertMenu(hMenu, 0xFFFFFFFF,
        MF_BYPOSITION | MF_STRING,
        IDM_EXIT, L"Exit");

    InsertMenu(hMenu, 0xFFFFFFFF,
        MF_BYPOSITION | MF_STRING,
        IDM_TOGGLEPAUSE, L"Pause");

    CheckMenuItem(hMenu, IDM_TOGGLEPAUSE,
        gTimer->Paused() ? MF_CHECKED : MF_UNCHECKED);

    if (!gTimer->Paused())
    {
        WCHAR mnItemBuf[100];
        if (gTimer->Delayed())
        {
            UINT days, hours, minutes;
            gTimer->DaysHrsMinDelayed(&days, &hours, &minutes);

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
            gTimer->Delayed() ? MF_CHECKED : MF_UNCHECKED);
    }

    POINT pt;
    GetCursorPos(&pt);
    POINT ptMenu = KeepPointInRect(pt, WorkAreaFromPoint(pt));
    WellBehavedTrackPopup(hwndTray, hMenu, ptMenu);
}

bool SetUpTrayWindow(HWND hwnd)
{
    hwndTray = hwnd;

    // Register the window with the shell
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_SHOWTIP | NIF_TIP;
    nid.uCallbackMessage = WM_USER_SHELLICON;
    nid.uID = nidID;
    if (!Shell_NotifyIcon(NIM_ADD, &nid))
    {
        Error(L"Shell_NotifyIcon failed!");
        return false;
    }

    // Create the timer
    gTimer = new CTimer(hwnd, UpdateIconAndTooltip, InjectBogusKeyboardInput);
    if (gTimer == nullptr)
    {
        Error(L"Couldn't create CTimer!");
        return false;
    }

    UpdateIconAndTooltip();
    return true;
}

LRESULT CALLBACK TrayWindowWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        if (!SetUpTrayWindow(hwnd))
        {
            return -1;
        }
        break;

    case WM_TIMER:
        gTimer->Callback(LOWORD(wParam));
        break;

    case WM_USER_SHELLICON:
        switch (LOWORD(lParam))
        {
        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
            if (LOWORD(lParam) == WM_RBUTTONDOWN)
            {
                RightClickMenu();
            }
            else
            {
                ShowPreviewWindow();
            }
            break;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_EXIT:
            delete gTimer;
            Shell_NotifyIcon(NIM_DELETE, &nid);
            PostQuitMessage(0);
            break;

        case IDM_TOGGLEPAUSE:
            gTimer->TogglePaused();
            break;

        case IDM_TOGGLE5HRDELAY:
            gTimer->ToggleDelay();
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

