
#include "MyKeepAlive.h"
using namespace std;

const SIZE WindowSize       = { 300, 150 };
const int FontSize          = 30;
const int FontSizeSm        = 15;

HWND ghwnd                  = nullptr;
HFONT ghFont                = nullptr;
HFONT ghFontSm              = nullptr;
UINT gdpi                   = 96;

#define DPISCALE(val)       MulDiv(val, gdpi, 96)

void UpdatePreviewWindow()
{
    InvalidateRect(ghwnd, nullptr, true);
}

void DrawPreviewWindow(HDC hdc, HWND hwnd)
{
    SetBkMode(hdc, TRANSPARENT);

    // Default (black) text if running, white text if paused
    if (!IsPaused())
    {
        SetTextColor(hdc, RGB(255, 255, 255));
    }

    // Green background if running, yellow if paused
    static HBRUSH hbrActive = CreateSolidBrush(RGB(1, 115, 18));
    static HBRUSH hbrInactive = CreateSolidBrush(RGB(199, 164, 10));
    HBRUSH hbr = IsPaused() ? hbrInactive : hbrActive;

    // Paint background
    RECT rc;
    GetClientRect(hwnd, &rc);
    FillRect(hdc, &rc, hbr);

    // Draw main text
    LPCWSTR szHeader = L"Keep Alive";
    SelectFont(hdc, ghFont);
    rc.top += DPISCALE(WindowSize.cy / 3);
    DrawText(hdc, szHeader, (int)wcslen(szHeader), &rc, DT_CENTER);

    // Draw sub text
    LPCWSTR szStatus = IsPaused() ? L"not running" : L"running";
    rc.top += DPISCALE(FontSize) + DPISCALE(5);
    SelectFont(hdc, ghFontSm);
    DrawText(hdc, szStatus, (int)wcslen(szStatus), &rc, DT_CENTER);
}

void EnsureFonts()
{
    // Only load fonts the first time called or when dpi changes
    static UINT dpiLast = 0;
    if (dpiLast == gdpi)
    {
        return;
    }
    dpiLast = gdpi;

    // Create large and small font (and destroy previous fonts)
    const LPCWSTR FontName = L"Courier New";
    ReCreateFont(&ghFont, DPISCALE(FontSize), FontName);
    ReCreateFont(&ghFontSm, DPISCALE(FontSizeSm), FontName);
}

void ShowHidePreviewWindow(bool show)
{
    // Remember last hide time, ignore shows immediatly following a hide.
    static ULONGLONG lasthide = 0;
    if (!show)
    {
        lasthide = GetTickCount64();
    }
    else if ((GetTickCount64() - lasthide) < 150)
    {
        // Note: this is for clicking on the tray button several times.
        // The click causes a de-activation (hide) and a show (click).
        // Dropping the show here changes clicking on the tray from a show
        // (and a flash if clicked multiple times) to a toggle.
        return;
    }

    // If hide, hide and return
    if (!show)
    {
        ShowWindow(ghwnd, SW_HIDE);
        return;
    }

    // When shown, window is positioned and sized around current cursor position.
    POINT ptCursor;
    GetCursorPos(&ptCursor);

    // Update dpi scale and fonts
    gdpi = DpiFromPt(ptCursor);
    EnsureFonts();

    // Start with rect centered around cursor, and move to be entirely in work area
    const SIZE sz = { DPISCALE(WindowSize.cx), DPISCALE(WindowSize.cy) };
    RECT rc = { ptCursor.x - (sz.cx / 2),
        ptCursor.y - (sz.cy / 2),
        ptCursor.x + (sz.cx / 2),
        ptCursor.y + (sz.cy / 2) };
    rc = KeepRectInRect(rc, WorkAreaFromPoint(ptCursor));

    // Position and show window
    SetWindowPos(ghwnd, 0, rc.left, rc.top, sz.cx, sz.cy, SWP_SHOWWINDOW);

    // Ensure window in foreground
    SetForegroundWindow(ghwnd);
}

LRESULT CALLBACK PreviewWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        ghwnd = hwnd;

        // Start hidden
        ShowHidePreviewWindow(false);
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        DrawPreviewWindow(hdc, hwnd);
        EndPaint(hwnd, &ps);
        break;
    }

    // Clicking anywhere on the preview window toggles pause
    case WM_LBUTTONDOWN:
        ToggleIsPaused();
        break;

    // When losing activation, hide
    case WM_ACTIVATE:
        if (wParam == WA_INACTIVE)
        {
            ShowHidePreviewWindow(false);
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

    if (CreateWindowEx(
            WS_EX_TOOLWINDOW,
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
