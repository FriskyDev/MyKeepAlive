
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

const SIZE PreviewSize = { 300, 150 };
const COLORREF rgbBackground = RGB(104, 197, 255);
LPCWSTR FontName = L"Courier New";
const int FontSize = 20;
const int FontSizeSm = 15;

HWND hwndPreview = nullptr;
UINT dpi = 96;
bool PreviewShowing = false;

#define STRLEN(str) (int)wcslen(str)
#define DPISCALE(val, dpi) MulDiv(val, dpi, 96)

void DrawPreviewText(HDC hdc, RECT rc, HFONT hfont, HFONT hfontSm)
{
    InflateRect(&rc, -10, -10);

    SelectFont(hdc, hfont);
    LPCWSTR szHeader = L"Keep Alive";
    DrawText(hdc, szHeader, STRLEN(szHeader), &rc, DT_CENTER);

    rc.top += DPISCALE(FontSize, dpi) + DPISCALE(5, dpi);
    SelectFont(hdc, hfontSm);
    LPCWSTR szSubHeader = L"Injects f-24 key every\n10 seconds to simulate input.";
    DrawText(hdc, szSubHeader, STRLEN(szSubHeader), &rc, DT_LEFT);

    rc.top += 2 * DPISCALE(FontSizeSm, dpi) + DPISCALE(10, dpi);
    LPCWSTR szStatusHeader = L"Current Status:";
    DrawText(hdc, szStatusHeader, STRLEN(szStatusHeader), &rc, DT_LEFT);
    rc.top += DPISCALE(FontSizeSm, dpi);

    WCHAR StatusBuf[100];

    if (paused)
    {
        swprintf((LPWSTR)&StatusBuf, 100, L"Inactive");
    }
    else if (DelayRemainingM >= 0)
    {
        rc.top += DPISCALE(10, dpi);
        swprintf((LPWSTR)&StatusBuf, 100,
            L"Active, set to go inactive\nin %i hr %i min",
            (DelayRemainingM / 60),
            DelayRemainingM - ((DelayRemainingM / 60) * 60));
    }
    else
    {
        swprintf((LPWSTR)&StatusBuf, 100, L"Running");
    }

    DrawText(hdc, (LPWSTR)&StatusBuf, STRLEN((LPWSTR)&StatusBuf), &rc, DT_LEFT);
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
    static HFONT hfontSm = nullptr;
    if (dpiLast != dpi)
    {
        dpiLast = dpi;

        if (hfont != nullptr)
        {
            DeleteFont(hfont);
        }

        hfont = CreateFont(
            DPISCALE(FontSize, dpi),
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            FontName);

        if (hfontSm != nullptr)
        {
            DeleteFont(hfontSm);
        }

        hfontSm = CreateFont(
            DPISCALE(FontSizeSm, dpi),
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            FontName);
    }

    if (hfont == nullptr || hfontSm == nullptr)
    {
        Error(L"Draw preview window failed to create fonts.");
    }

    SetBkMode(hdc, TRANSPARENT);
    DrawPreviewText(hdc, rc, hfont, hfontSm);
}

RECT ShowPreviewWindow(POINT pt)
{
    dpi = DpiFromPt(pt);
    SIZE sz = { DPISCALE(PreviewSize.cx, dpi), DPISCALE(PreviewSize.cy, dpi) };

    RECT rc = { pt.x, pt.y, pt.x + sz.cx, pt.y + sz.cy };
    rc = KeepRectInRect(rc, WorkAreaFromPoint(pt));

    SetWindowPos(hwndPreview, nullptr,
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        SWP_SHOWWINDOW);

    SetForegroundWindow(hwndPreview);

    PreviewShowing = true;
    return rc;
}

void HidePreviewWindow()
{
    SetWindowPos(hwndPreview, nullptr, 0, 0 ,0 , 0,
        SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE);

    PreviewShowing = false;
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

    case WM_ACTIVATE:
        if (wParam == WA_INACTIVE)
        {
            HidePreviewWindow();
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

    if (RegisterClassEx(&wcex) == 0) {
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
