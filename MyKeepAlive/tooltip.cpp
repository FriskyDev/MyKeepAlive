
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

const SIZE TooltipSize = { 250, 50 };
LPCWSTR FontName = L"Courier New";
const int FontSize = 20;
UINT dpi = 96;
COLORREF rgbBackground = RGB(104, 197, 255);
bool TooltipShowing = false;

void DrawTooltipText(HDC hdc, RECT rc)
{
    // TODO: gotta be something better, maybe replace all with default tooltip??
    DrawText(hdc, L"Keep Alive!", 11, &rc, DT_VCENTER | DT_SINGLELINE | DT_CENTER);
}

void Draw(HDC hdc, HWND hwnd)
{
    // fill background color
    static HBRUSH hbr = CreateSolidBrush(rgbBackground);
    RECT rc;
    GetClientRect(hwnd, &rc);
    FillRect(hdc, &rc, hbr);

    // cache font for the last DPI
    static UINT dpiLast = 0;
    static HFONT hfont = nullptr;
    if (dpiLast != dpi || hfont == nullptr)
    {
        dpiLast = dpi;

        hfont = CreateFont(
            MulDiv(FontSize, dpi, 96),
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            FontName);
    }

    SetBkMode(hdc, TRANSPARENT);
    SelectFont(hdc, hfont);
    DrawTooltipText(hdc, rc);
}

RECT ShowHoverTooltip(POINT pt)
{
    dpi = DpiFromPt(pt);
    SIZE sz = { MulDiv(TooltipSize.cx, dpi, 96), MulDiv(TooltipSize.cy, dpi, 96) };

    RECT rcStart = { pt.x, pt.y, pt.x + sz.cx, pt.y + sz.cy };
    RECT rc = KeepRectInRect(rcStart, WorkAreaFromPoint(pt));

    SetWindowPos(hwndTooltip, nullptr,
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        SWP_SHOWWINDOW);

    SetForegroundWindow(hwndTooltip);

    TooltipShowing = true;
    return rc;
}

void HideHoverTooltip()
{
    SetWindowPos(hwndTooltip, nullptr, 0, 0 ,0 , 0,
        SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE);

    TooltipShowing = false;
}

LRESULT CALLBACK TooltipWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        Draw(hdc, hwnd);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_ACTIVATE:
        if (wParam == WA_INACTIVE)
        {
            HideHoverTooltip();
        }
        break;

    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}
