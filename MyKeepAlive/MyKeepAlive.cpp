
#include "stdafx.h"
#include <windows.h>
#include "Windowsx.h"
#include "Shellapi.h""
#include <tchar.h>
#include <string>
#include "resource.h"
using namespace std;

#define IDT_TIMER 104
#define IDM_EXIT 105
#define IDM_TOGGLEPAUSE 106
#define	WM_USER_SHELLICON WM_USER + 1

HINSTANCE hInst = nullptr;
LPCWSTR szTitle = L"keepalive";
LPCWSTR szWindowClass = L"keepalive_class";
HWND hwnd = nullptr;
HMENU hMenu = nullptr;
const UINT TimerMS = 10000; // 10 seconds
bool paused = false;

__inline void Error(std::wstring msg) {
    MessageBox(nullptr, msg.c_str(), _T("Error"), MB_OK); }


void TickTimer()
{
    if (!paused)
    {
        static INPUT i = {
            INPUT_KEYBOARD ,
            { VK_F24, 0 /*wScan*/, 0 /*dwFlags*/, 0 /*time*/, 0 /*dwExtraInfo*/ }
        };
        SendInput(1, &i, sizeof(INPUT));
    }
}

void ShowPopupMenu(HWND hwnd)
{
    hMenu = CreatePopupMenu();

    InsertMenu(hMenu, 0xFFFFFFFF,
        MF_BYPOSITION | MF_STRING,
        IDM_EXIT, L"Exit");

    InsertMenu(hMenu, 0xFFFFFFFF,
        MF_BYPOSITION | MF_STRING,
        IDM_TOGGLEPAUSE, paused ? L"Resume" : L"Pause");

    POINT pt;
    GetCursorPos(&pt);

    TrackPopupMenu(hMenu, // TODO: how to dismiss more cleanly...
        TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN,
        pt.x, pt.y, 0, hwnd, nullptr);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        SetTimer(hwnd, IDT_TIMER, TimerMS, nullptr);
        break;
    case WM_TIMER:
        if (wParam == IDT_TIMER)
        {
            TickTimer();
        }
        break;

    case WM_USER_SHELLICON:
    {
        switch (LOWORD(lParam))
        {
        case WM_RBUTTONDOWN:
            ShowPopupMenu(hwnd);
            break;

        // TODO: do something on left click/ hover,
        //       maybe show last inject/ count, something...
        }
    }
    break;
    
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDM_EXIT:
            // TODO: refresh taskbar so icon goes away smoothly
            PostQuitMessage(0);
            break;

        case IDM_TOGGLEPAUSE:
            paused = !paused;
            break;
        }
    }
    break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE,
                      _In_ LPWSTR,
                      _In_ int)
{
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYKEEPALIVE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = nullptr;
    if (RegisterClassEx(&wcex) == 0)
    {
        Error(L"RegisterClass failed!");
        return 1;
    }

    HWND hwnd = CreateWindow(
        szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, hInstance, nullptr);

    if (hwnd == nullptr)
    {
        Error(L"CreateWindow failed!");
        return 1;
    }

    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uFlags = NIF_ICON | NIF_MESSAGE;
    nid.uID = IDI_MYKEEPALIVE;
    nid.hIcon = LoadIcon(hInstance,
        (LPCTSTR)MAKEINTRESOURCE(IDI_MYKEEPALIVE));
    nid.uCallbackMessage = WM_USER_SHELLICON;
    if (!Shell_NotifyIcon(NIM_ADD, &nid))
    {
        Error(L"Shell_NotifyIcon failed!");
        return 1;
    }
    
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
