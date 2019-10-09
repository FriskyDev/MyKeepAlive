
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

void Error(std::wstring msg)
{
    MessageBox(nullptr, msg.c_str(), _T("Error"), MB_OK);
    PostQuitMessage(1);
}
void Warning(std::wstring msg)
{
    MessageBox(nullptr, msg.c_str(), _T("Warning"), MB_OK);
}

RECT WorkAreaFromPoint(POINT pt)
{
    MONITORINFOEX mi;
    mi.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST), &mi);
    return mi.rcWork;
}

UINT DpiFromPt(POINT pt)
{
    UINT dpi;
    GetDpiForMonitor(
        MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST),
        MDT_EFFECTIVE_DPI, &dpi, &dpi);
    return dpi;
}

POINT KeepPointInRect(POINT pt, RECT rc)
{
    return {
        min(max(pt.x, rc.left), rc.right),
        min(max(pt.x, rc.top), rc.bottom)
    };
}

RECT KeepRectInRect(RECT rcStartRect, RECT rcBounds)
{
    if (rcStartRect.top < rcBounds.top)
    {
        OffsetRect(&rcStartRect, 0, rcBounds.top - rcStartRect.top);
    }
    if (rcStartRect.bottom > rcBounds.bottom)
    {
        OffsetRect(&rcStartRect, 0, rcBounds.bottom - rcStartRect.bottom);
    }
    if (rcStartRect.left < rcBounds.left)
    {
        OffsetRect(&rcStartRect, rcBounds.left - rcStartRect.left, 0);
    }
    if (rcStartRect.right > rcBounds.right)
    {
        OffsetRect(&rcStartRect, rcBounds.right - rcStartRect.right, 0);
    }

    return rcStartRect;
}

void WellBehavedTrackPopup(HWND hwnd, HMENU hMenu, POINT pt)
{
    SetForegroundWindow(hwnd);

    TrackPopupMenu(hMenu,
        TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN,
        pt.x, pt.y, 0, hwnd, nullptr);

    PostMessage(hwnd, WM_NULL, 0, 0);
}

void ReCreateFont(HFONT* phFont, UINT size, LPCWSTR fontName)
{
    if (*phFont != nullptr)
    {
        DeleteFont(*phFont);
    }
    *phFont = CreateFont(size, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, fontName);
    if (*phFont == nullptr)
    {
        Error(L"Draw preview window failed to create fonts.");
    }
}