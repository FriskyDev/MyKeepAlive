
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

void Error(std::wstring msg)
{
    MessageBox(nullptr, msg.c_str(), _T("Error"), MB_OK);
}

RECT WorkAreaFromPoint(POINT pt)
{
    MONITORINFOEX mi;
    mi.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST), &mi);
    return mi.rcWork;
}

POINT KeepPointInRect(POINT pt, RECT rc)
{
    return{ min(max(pt.x, rc.left), rc.right),
        min(max(pt.x, rc.top), rc.bottom) };
}


POINT ptCursorPosClippedToWorkArea()
{
    POINT pt;
    GetCursorPos(&pt);
    return KeepPointInRect(pt, WorkAreaFromPoint(pt));
}

RECT rcCursorPosClippedToWorkArea(int cx, int cy)
{
    POINT pt;
    GetCursorPos(&pt);
    RECT rc = { pt.x, pt.y, cx, cy };
    RECT rcWork = WorkAreaFromPoint(pt);
    if (rc.top < rcWork.top)
    {
        OffsetRect(&rc, 0, rcWork.top - rc.top);
    }
    if (rc.bottom > rcWork.bottom)
    {
        OffsetRect(&rc, 0, rc.bottom - rcWork.bottom);
    }
    if (rc.left < rcWork.left)
    {
        OffsetRect(&rc, 0, rcWork.left - rc.left);
    }
    if (rc.right > rcWork.right)
    {
        OffsetRect(&rc, 0, rc.right - rcWork.right);
    }
    return rc;
}

void WellBehavedTrackPopup(HWND hwnd, HMENU hMenu, POINT pt)
{
    SetForegroundWindow(hwnd);
    TrackPopupMenu(hMenu,
        TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN,
        pt.x, pt.y, 0, hwnd, nullptr);
    PostMessage(hwnd, WM_NULL, 0, 0);
}
