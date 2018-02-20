
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

const SIZE PreviewSize = { 300, 150 };
const COLORREF rgbBackground = RGB(104, 197, 255);
LPCWSTR FontName = L"Courier New";
const int FontSize = 20;
const int FontSizeSm = 15;

HWND hwndPreview = nullptr;
bool PreviewShowing = false;

UINT dpi = 96;
#define DPISCALE(val) MulDiv(val, dpi, 96)

void DrawPreviewText(HDC hdc, RECT rc, HFONT hfont, HFONT hfontSm)
{
    // Draw the preview window. Fonts and background already done.

    InflateRect(&rc, DPISCALE(-10) , DPISCALE(-10));

    SelectFont(hdc, hfont);
    LPCWSTR szHeader = L"Keep Alive";
    DrawText(hdc, szHeader, STRLEN(szHeader), &rc, DT_CENTER);
    rc.top += DPISCALE(FontSize) + DPISCALE(5);

    SelectFont(hdc, hfontSm);
    LPCWSTR szSubHeader = L"Injects f-24 key every\n10 seconds to simulate input.";
    DrawText(hdc, szSubHeader, STRLEN(szSubHeader), &rc, DT_LEFT);
    rc.top += 2 * DPISCALE(FontSizeSm) + DPISCALE(10);

    LPCWSTR szStatusHeader = L"Current Status:";
    DrawText(hdc, szStatusHeader, STRLEN(szStatusHeader), &rc, DT_LEFT);
    rc.top += DPISCALE(FontSizeSm);

    WCHAR StatusBuf[100];

    if (paused)
    {
        swprintf(BUFSTR(StatusBuf), 100, L"Inactive");
    }
    else if (DelayRemainingM >= 0)
    {
        UINT days, hours, minutes;
        DaysMinsSecsFromMinutes(DelayRemainingM, &days, &hours, &minutes);

        swprintf(BUFSTR(StatusBuf), 100,
            L"Will pause in %i hr %i min", hours, minutes);
    }
    else
    {
        swprintf(BUFSTR(StatusBuf), 100, L"Running");
    }

    DrawText(hdc, BUFSTR(StatusBuf), STRLEN(BUFSTR(StatusBuf)), &rc, DT_LEFT);
    rc.top += DPISCALE(10);

    if (TotalTimeRunningM > 0)
    {
        UINT days, hours, minutes;
        DaysMinsSecsFromMinutes(TotalTimeRunningM, &days, &hours, &minutes);

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
}

void GetFontsFromCache(UINT dpi, _Out_ HFONT* hfont, _Out_ HFONT* hfontSm)
{
    static UINT dpiLast = 0;
    static HFONT hfontLast = nullptr;
    static HFONT hfontSmLast = nullptr;

    // Load fonts once for each DPI, throwing out the previously created fonts
    if (dpiLast != dpi)
    {
        dpiLast = dpi;

        if (hfontLast != nullptr)
        {
            DeleteFont(hfontLast);
        }

        hfontLast = CreateFont(
            DPISCALE(FontSize),
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            FontName);

        if (hfontSmLast != nullptr)
        {
            DeleteFont(hfontSmLast);
        }

        hfontSmLast = CreateFont(
            DPISCALE(FontSizeSm),
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            FontName);
    }

    // We expect creating fonts to always succeed
    if (hfont == nullptr || hfontSm == nullptr)
    {
        Error(L"Draw preview window failed to create fonts.");
    }

    *hfont = hfontLast;
    *hfontSm = hfontSmLast;
}

void Draw(HDC hdc, HWND hwnd)
{
    // Fill background color
    static HBRUSH hbr = CreateSolidBrush(rgbBackground);
    RECT rc;
    GetClientRect(hwnd, &rc);
    FillRect(hdc, &rc, hbr);

    // Get/ create fonts for the current dpi
    HFONT hfont, hfontSm;
    GetFontsFromCache(dpi, &hfont, &hfontSm);

    // Do text stuff
    SetBkMode(hdc, TRANSPARENT);
    DrawPreviewText(hdc, rc, hfont, hfontSm);
}

void ShowPreviewWindow()
{
    if (PreviewShowing)
    {
        return;
    }

    // When opened, will be positioned near the cursor
    // todo: can i get the position of the icon itself?
    POINT pt;
    GetCursorPos(&pt);

    // From the anchor point, set dpi (the scale factor for our monitor)
    dpi = DpiFromPt(pt);

    // Find the size, and then the final position (the anchor point, extended
    // by size, strictly within the work area).
    SIZE sz = { DPISCALE(PreviewSize.cx), DPISCALE(PreviewSize.cy) };
    RECT rc = { pt.x, pt.y, pt.x + sz.cx, pt.y + sz.cy };
    rc = KeepRectInRect(rc, WorkAreaFromPoint(pt));

    // Position and show window
    SetWindowPos(hwndPreview, nullptr,
        rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
        SWP_SHOWWINDOW);

    // Give window foreground to ensure we get a de-activate (to know when to hide)
    SetForegroundWindow(hwndPreview);

    PreviewShowing = true;
}

void HidePreviewWindow()
{
    if (PreviewShowing)
    {
        SetWindowPos(hwndPreview, nullptr, 0, 0, 0, 0,
            SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE);

        PreviewShowing = false;
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
