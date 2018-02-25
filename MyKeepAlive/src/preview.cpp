
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

const SIZE PreviewSize = { 300, 150 };
const COLORREF rgbBackground = RGB(91, 224, 76);
const COLORREF rgbBackgroundPaused = RGB(242, 207, 31);
const LPCWSTR FontName = L"Courier New";
const int FontSize = 20;
const int FontSizeSm = 15;

HWND hwndPreview = nullptr;
HFONT hfont = nullptr;
HFONT hfontSm = nullptr;

UINT dpi = 96;
#define DPISCALE(val) MulDiv(val, dpi, 96)

void Draw(HDC hdc, HWND hwnd)
{
    SetBkMode(hdc, TRANSPARENT);

    static HBRUSH hbrActive = CreateSolidBrush(rgbBackground);
    static HBRUSH hbrInactive = CreateSolidBrush(rgbBackgroundPaused);

    RECT rc;
    GetClientRect(hwnd, &rc);
    FillRect(hdc, &rc, Paused() ? hbrInactive : hbrActive);

    InflateRect(&rc, DPISCALE(-10) , DPISCALE(-10));

    SelectFont(hdc, hfont);
    LPCWSTR szHeader = L"Keep Alive";
    DrawText(hdc, szHeader, STRLEN(szHeader), &rc, DT_CENTER);
    rc.top += DPISCALE(FontSize) + DPISCALE(5);

    SelectFont(hdc, hfontSm);
    LPCWSTR szSubHeader = L"Injects f-24 key every\n10 seconds to simulate input.";
    DrawText(hdc, szSubHeader, STRLEN(szSubHeader), &rc, DT_LEFT);
    rc.top += 2 * DPISCALE(FontSizeSm) + DPISCALE(10);

    WCHAR StatusBuf[100];
    if (Paused())
    {
        swprintf(BUFSTR(StatusBuf), 100, L"Status: Inactive\n(NOT injecting input)");
    }
    else
    {
        swprintf(BUFSTR(StatusBuf), 100, L"Status: Running\n(injecting ghost input)");
    }

    DrawText(hdc, BUFSTR(StatusBuf), STRLEN(BUFSTR(StatusBuf)), &rc, DT_LEFT);
    rc.top += DPISCALE(10);

    UINT days, hours, minutes;
    DaysHrsMinTotal(&days, &hours, &minutes);

    WCHAR TotalTimeBuf[100];
    if (days > 0)
    {
        swprintf(BUFSTR(TotalTimeBuf), 100,
            L"%i days, %i hours, %i min",
            days, hours, minutes);
    }
    else if (hours > 0)
    {
        swprintf(BUFSTR(TotalTimeBuf), 100,
            L"%i hours, %i min", hours, minutes);
    }
    else
    {
        swprintf(BUFSTR(TotalTimeBuf), 100,
            L"%i min", minutes);
    }

    DrawText(hdc, BUFSTR(TotalTimeBuf), STRLEN(BUFSTR(TotalTimeBuf)),
        &rc, DT_RIGHT | DT_BOTTOM | DT_SINGLELINE);
}

void RefreshFontsForDpi(UINT dpi)
{
    static UINT dpiLast = 0;
    if (dpiLast != dpi)
    {
        dpiLast = dpi;

        if (hfont != nullptr)
        {
            DeleteFont(hfont);
        }

        hfont = CreateFont(
            DPISCALE(FontSize),
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            FontName);

        if (hfontSm != nullptr)
        {
            DeleteFont(hfontSm);
        }

        hfontSm = CreateFont(
            DPISCALE(FontSizeSm),
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            FontName);
    }

    if (hfont == nullptr || hfontSm == nullptr)
    {
        Error(L"Draw preview window failed to create fonts.");
    }
}

void ShowHidePreview(bool show)
{
    if (show)
    {
        POINT pt;
        GetCursorPos(&pt);
        dpi = DpiFromPt(pt);
        RefreshFontsForDpi(dpi);

        RECT rc = { pt.x, pt.y,
            pt.x + DPISCALE(PreviewSize.cx),
            pt.y + DPISCALE(PreviewSize.cy) };
        rc = KeepRectInRect(rc, WorkAreaFromPoint(pt));
        SetWindowPos(hwndPreview, nullptr,
            rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
            SWP_SHOWWINDOW);

        SetForegroundWindow(hwndPreview);
    }
    else
    {
        SetWindowPos(hwndPreview, nullptr, 0, 0, 0, 0,
            SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE);
    }

}

LRESULT CALLBACK PreviewWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        hwndPreview = hwnd;
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        Draw(hdc, hwnd);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_LBUTTONDOWN:
        TogglePaused();
        InvalidateRect(hwnd, nullptr, true);
        break;

    case WM_ACTIVATE:
        if (wParam == WA_INACTIVE)
        {
            ShowHidePreview(false);
        }
        break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

bool CreatePreviewWindow()
{
    const LPCWSTR szPreviewWindowClass = L"keepalive_class_previewwnd";

    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_DBLCLKS;
    wcex.lpfnWndProc = PreviewWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szPreviewWindowClass;
    wcex.hIconSm = NULL;

    if (RegisterClassEx(&wcex) == 0)
    {
        Error(L"Registering Preview Window Failed");
        return FALSE;
    }

    if (CreateWindowEx(WS_EX_TOOLWINDOW,
                       szPreviewWindowClass,
                       L"",
                       WS_POPUP,
                       0, 0, 0, 0,
                       nullptr, nullptr, hInstance, nullptr) == nullptr)
    {
        Error(L"Create Preview Window failed!");
        return false;
    }

    return true;
}