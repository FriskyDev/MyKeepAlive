
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

LRESULT CALLBACK TooltipWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, SWP_HIDEWINDOW);
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        static HBRUSH hbr = CreateSolidBrush(RGB(0, 0, 255));
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, hbr);
        EndPaint(hwnd, &ps);
    }
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

