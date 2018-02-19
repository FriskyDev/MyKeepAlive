
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

HWND hwndPreview = nullptr;
const SIZE PreviewSize = { 250, 50 };
LPCWSTR FontName = L"Courier New";
const int FontSize = 20;
UINT dpi = 96;
COLORREF rgbBackground = RGB(104, 197, 255);
bool PreviewShowing = false;

void DrawPreviewText(HDC hdc, RECT rc)
{
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
    DrawPreviewText(hdc, rc);
}

RECT ShowPreviewWindow(POINT pt)
{
    dpi = DpiFromPt(pt);
    SIZE sz = { MulDiv(PreviewSize.cx, dpi, 96), MulDiv(PreviewSize.cy, dpi, 96) };

    RECT rcStart = { pt.x, pt.y, pt.x + sz.cx, pt.y + sz.cy };
    RECT rc = KeepRectInRect(rcStart, WorkAreaFromPoint(pt));

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
