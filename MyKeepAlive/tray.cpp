
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

NOTIFYICONDATA nid = {};
HWND hwndTray = nullptr;
const UINT TimerMS = 10000;          // 10 seconds
const UINT LongTimerMS = 60000;      // 1 minute
const UINT DelayTimeoutM = 60 * 5;   // 5 hours
int DelayRemainingM = -1;

void UpdateTooltipText()
{
    if (paused)
    {
        wcscpy_s(nid.szTip, L"Keep Alive - Paused");
    }
    else if (DelayRemainingM >= 0)
    {
        int hours = (DelayRemainingM / 60);
        int min = DelayRemainingM - ((DelayRemainingM / 60) * 60);

        swprintf(nid.szTip, 128,
                 L"Keep Alive\nWill pause in %i hr %i min",
                 (DelayRemainingM / 60),
                 DelayRemainingM - ((DelayRemainingM / 60) * 60));
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

void ShowRightClickMenu(HWND hwnd)
{
    HMENU hMenu = CreatePopupMenu();

    InsertMenu(hMenu, 0xFFFFFFFF,
        MF_BYPOSITION | MF_STRING,
        IDM_EXIT, L"Exit");

    InsertMenu(hMenu, 0xFFFFFFFF,
        MF_BYPOSITION | MF_STRING,
        IDM_TOGGLEPAUSE, L"Pause");

    CheckMenuItem(hMenu, IDM_TOGGLEPAUSE,
        paused ? MF_CHECKED : MF_UNCHECKED);

    if (!paused)
    {
        InsertMenu(hMenu, 0xFFFFFFFF,
            MF_BYPOSITION | MF_STRING,
            IDM_TOGGLE5HRDELAY, L"Pause in 5 hrs");
        // TODO: if on delay, replace text with 'pause in X hr Y m'

        CheckMenuItem(hMenu, IDM_TOGGLE5HRDELAY,
            DelayRemainingM >= 0 ? MF_CHECKED : MF_UNCHECKED);
    }

    // Open the menu at the cursor pos, but within the work area
    POINT pt;
    GetCursorPos(&pt);
    POINT ptMenu = KeepPointInRect(pt, WorkAreaFromPoint(pt));
    WellBehavedTrackPopup(hwnd, hMenu, ptMenu);

    UpdateTooltipText();
}

void Click(bool rightClick)
{
    if (rightClick)
    {
        ShowRightClickMenu(hwndTray);
    }
    else
    {
        // TODO: for now disable preview window
        //if (!PreviewShowing)
        //{
        //    POINT pt;
        //    GetCursorPos(&pt);
        //    ShowPreviewWindow(pt);
        //}
    }

    UpdateTooltipText();
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

    UpdateTooltipText();
}

void Tick(bool LongTimer)
{
    if (LongTimer) /* IDT_TIMER_LONG, fires every minute */
    {
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
    nid.uID = IDI_HANDPIC;
    nid.hIcon = LoadIcon(hInstance,
        (LPCTSTR)MAKEINTRESOURCE(IDI_HANDPIC));
    nid.uCallbackMessage = WM_USER_SHELLICON;

    if (!Shell_NotifyIcon(NIM_ADD, &nid))
    {
        Error(L"Shell_NotifyIcon failed!");
        return false;
    }

    UpdateTooltipText();
    return true;
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
            PostQuitMessage(0);
            Shell_NotifyIcon(NIM_DELETE, &nid);
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
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HANDPIC));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
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

