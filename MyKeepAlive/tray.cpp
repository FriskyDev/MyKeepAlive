
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

const UINT TimerMS = 10000;     // 10 seconds
const UINT LongTimerMS = 60000; // 1 minute
bool paused = false;
int MinToAutoPause = -1;

void InjectBogusKeyboardInput()
{
    static INPUT i = { INPUT_KEYBOARD, { VK_F24, 0, 0, 0, 0 } };
    SendInput(1, &i, sizeof(INPUT));
}

POINT GetRightClipMenuPt()
{
    POINT pt;
    GetCursorPos(&pt);
    return KeepPointInRect(pt, WorkAreaFromPoint(pt));
}

void ShowRightClickMenu(HWND hwnd)
{
    HMENU hMenu = CreatePopupMenu();

    InsertMenu(hMenu, 0xFFFFFFFF,
        MF_BYPOSITION | MF_STRING,
        IDM_EXIT, L"Exit");

    InsertMenu(hMenu, 0xFFFFFFFF,
        MF_BYPOSITION | MF_STRING,
        IDM_TOGGLEPAUSE, paused ? L"Resume" : L"Pause");

    InsertMenu(hMenu, 0xFFFFFFFF,
        MF_BYPOSITION | MF_STRING,
        IDM_PAUSE_IN_5HRS, L"5 HR Pause");

    CheckMenuItem(hMenu, IDM_PAUSE_IN_5HRS,
        MinToAutoPause >= 0 ? MF_CHECKED : MF_UNCHECKED);

    WellBehavedTrackPopup(hwnd,
        hMenu,
        GetRightClipMenuPt());
}

LRESULT CALLBACK TrayWindowWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        SetTimer(hwnd, IDT_TIMER, TimerMS, nullptr);
        SetTimer(hwnd, IDT_TIMER_LONG, LongTimerMS, nullptr);
        break;

    case WM_TIMER:
        switch (LOWORD(wParam))
        {
        case IDT_TIMER:
            InjectBogusKeyboardInput();
            break;

        case IDT_TIMER_LONG:
            if (MinToAutoPause == 0)
            {
                paused = true;
            }
            else if (MinToAutoPause > 0)
            {
                MinToAutoPause--;
            }
            break;
        }
        break;

    case WM_USER_SHELLICON:
        switch (LOWORD(lParam))
        {
        case WM_RBUTTONDOWN:
            ShowRightClickMenu(hwnd);
            break;
        //case WM_MOUSEMOVE: // TODO: this messes with right click menu... but should be on hover
            // maybe for hover, know if it's up, and ptinrect to close?
        case WM_LBUTTONDOWN:
            ShowHoverTooltip();
            break;
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDM_EXIT:
            PostQuitMessage(0);
            break;

        case IDM_TOGGLEPAUSE:
            paused = !paused;
            break;

        case IDM_PAUSE_IN_5HRS:
            if (MinToAutoPause < 0)
            {
                MinToAutoPause = 60 * 5;
            }
            else
            {
                MinToAutoPause = -1;
            }
            break;
        }
        break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

