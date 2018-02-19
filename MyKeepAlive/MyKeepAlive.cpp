
#include "stdafx.h"
#include <windows.h>
#include "Windowsx.h"
#include "Shellapi.h""
#include <tchar.h>
#include <string>
#include "resource.h"
using namespace std;

#define IDT_TIMER           101
#define IDT_TIMER_LONG      102
#define IDM_EXIT            111
#define IDM_TOGGLEPAUSE     112
#define IDM_PAUSE_IN_5HRS   113
#define	WM_USER_SHELLICON   WM_USER + 1

HINSTANCE hInst = nullptr;
LPCWSTR szTitle = L"keepalive";
LPCWSTR szWindowClass = L"keepalive_class";
HWND hwnd = nullptr;
HMENU hMenu = nullptr;
NOTIFYICONDATA nid = {};

const UINT TimerMS = 10000;     // 10 seconds
const UINT LongTimerMS = 60000; // 1 minute
bool paused = false;
int MinToAutoPause = -1;

void Error(std::wstring msg)
{
    MessageBox(nullptr, msg.c_str(), _T("Error"), MB_OK);
}

void InjectBogusKeyboardInput()
{
    static INPUT i = { INPUT_KEYBOARD , { VK_F24, 0, 0, 0, 0 } };
    SendInput(1, &i, sizeof(INPUT));
}

RECT rcWorkFromPt(POINT pt)
{    
    MONITORINFOEX mi;
    mi.cbSize = sizeof(MONITORINFOEX);
    GetMonitorInfo(MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST), &mi);
    return mi.rcWork;
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

    InsertMenu(hMenu, 0xFFFFFFFF,
        MF_BYPOSITION | MF_STRING,
        IDM_PAUSE_IN_5HRS, L"5 HR Pause");

    CheckMenuItem(hMenu, IDM_PAUSE_IN_5HRS,
        MinToAutoPause >= 0 ? MF_CHECKED : MF_UNCHECKED);

    POINT pt;
    GetCursorPos(&pt);
    RECT rcWork = rcWorkFromPt(pt);
    pt.x = min(max(pt.x, rcWork.left), rcWork.right);
    pt.y = min(max(pt.x, rcWork.top), rcWork.bottom);

    SetForegroundWindow(hwnd); // weird hack to dismiss menu when loses activation
    TrackPopupMenu(hMenu,
                   TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN,
                   pt.x, pt.y, 0, hwnd, nullptr);
    PostMessage(hwnd, WM_NULL, 0, 0);
}

void PopupMenuMessage(UINT message)
{
    switch (message)
    {
    case IDM_EXIT:
        PostQuitMessage(0);
        break;

    case IDM_TOGGLEPAUSE:
        paused = !paused;
        break;

    case IDM_PAUSE_IN_5HRS:
        if (MinToAutoPause < 0)
        {
            MinToAutoPause = 60 * 5;
        }
        else
        {
            MinToAutoPause = -1;
        }
        break;
    }
}

void TrayWindowInputMessage(UINT message)
{
    switch (message)
    {
    case WM_RBUTTONDOWN:
        ShowPopupMenu(hwnd);
        break;
    }
}

void HandleTimer(bool LongTimer)
{
    if (LongTimer)
    {
        if (MinToAutoPause == 0)
        {
            paused = true;
        }
        else if (MinToAutoPause > 0)
        {
            MinToAutoPause--;
        }
    }
    else
    {
        if (!paused)
        {
            InjectBogusKeyboardInput();
        }
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        SetTimer(hwnd, IDT_TIMER, TimerMS, nullptr);
        SetTimer(hwnd, IDT_TIMER_LONG, LongTimerMS, nullptr);
        break;

    case WM_TIMER:
        HandleTimer(wParam == IDT_TIMER_LONG);
        break;

    case WM_USER_SHELLICON:
        if (LOWORD(lParam) == WM_RBUTTONDOWN) {
            ShowPopupMenu(hwnd);
        }
        break;
    
    case WM_COMMAND:
        PopupMenuMessage(LOWORD(wParam));
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

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_SHOWTIP;
    nid.uID = IDI_MYKEEPALIVE;
    nid.hIcon = LoadIcon(hInstance,
        (LPCTSTR)MAKEINTRESOURCE(IDI_MYKEEPALIVE));
    nid.uCallbackMessage = WM_USER_SHELLICON;

    //wcscpy_s(nid.szTip, _TEXT("tooltip"));

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

    Shell_NotifyIcon(NIM_DELETE, &nid);
    return 0;
}
