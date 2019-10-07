
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

const SIZE PreviewSize = { 300, 150 };
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

    if (!Paused())
    {
        SetTextColor(hdc, RGB(255, 255, 255));
    }

    static HBRUSH hbrActive = CreateSolidBrush(RGB(1, 115, 18));
    static HBRUSH hbrInactive = CreateSolidBrush(RGB(199, 164, 10));

    RECT rc;
    GetClientRect(hwnd, &rc);
    FillRect(hdc, &rc, Paused() ? hbrInactive : hbrActive);

    SelectFont(hdc, hfont);
    LPCWSTR szHeader = L"Keep Alive";
    rc.top += DPISCALE(PreviewSize.cy / 3);
    DrawText(hdc, szHeader, STRLEN(szHeader), &rc, DT_CENTER);
    rc.top += DPISCALE(FontSize) + DPISCALE(5);

    WCHAR StatusBuf[100];
    if (Paused())
    {
        swprintf(BUFSTR(StatusBuf), 100, L"not running");
    }
    else
    {
        swprintf(BUFSTR(StatusBuf), 100, L"running");
    }

    SelectFont(hdc, hfontSm);
    DrawText(hdc, BUFSTR(StatusBuf), STRLEN(BUFSTR(StatusBuf)), &rc, DT_CENTER);
}

void RefreshFonts(UINT dpi)
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
		// Update dpi scale
        POINT pt;
        GetCursorPos(&pt);
        dpi = DpiFromPt(pt);
		RefreshFonts(dpi);

		// Position preview window and show
		const SIZE sz = { DPISCALE(PreviewSize.cx), DPISCALE(PreviewSize.cy) };
        RECT rc = { pt.x - (sz.cx / 2),
					pt.y - (sz.cy / 2),
		            pt.x + (sz.cx / 2),
					pt.y + (sz.cy / 2) };
        rc = KeepRectInRect(rc, WorkAreaFromPoint(pt));
        SetWindowPos(hwndPreview, nullptr,
            rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
            SWP_SHOWWINDOW);

		// Attempt to ensure in fg after created (because we track loss of fg to hide)
        SetForegroundWindow(hwndPreview);
    }
    else
    {
		// Hide the window
		ShowWindow(hwndPreview, SW_HIDE);
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
