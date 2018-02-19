
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

HINSTANCE hInst = nullptr;
LPCWSTR szTitle = L"keepalive";
LPCWSTR szWindowClass = L"keepalive_class";
LPCWSTR szWindowTooltipClass = L"keepalive_class_tooltip";
NOTIFYICONDATA nid = {};
HWND hwndTooltip = nullptr;

bool CreateTooltipWindow(HINSTANCE hInstance)
{
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_DBLCLKS;
    wcex.lpfnWndProc = TooltipWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = NULL;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowTooltipClass;
    wcex.hIconSm = NULL;
    if (RegisterClassEx(&wcex) == 0) {
        Error(L"Registering Main Window Failed");
        return FALSE;
    }

    HWND hwnd = CreateWindow(
        szWindowTooltipClass,
        L"",
        WS_POPUP ,
        0, 0, 0, 0,
        NULL,
        NULL,
        hInstance,
        NULL);

    if (hwnd == nullptr)
    {
        Error(L"Create Tooltip failed!");
        return false;
    }

    hwndTooltip = hwnd;
    return true;
}

bool CreateTrayWindow(HINSTANCE hInstance)
{
    WNDCLASSEX wcex = {};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wcex.lpfnWndProc = TrayWindowWndProc;
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
        Error(L"Register Tray failed!");
        return false;
    }

    HWND hwnd = CreateWindow(
        szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        nullptr, nullptr, hInstance, nullptr);

    if (hwnd == nullptr)
    {
        Error(L"Create Tray failed!");
        return false;
    }

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hwnd;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_SHOWTIP;
    nid.uID = IDI_MYKEEPALIVE;
    nid.hIcon = LoadIcon(hInstance,
        (LPCTSTR)MAKEINTRESOURCE(IDI_MYKEEPALIVE));
    nid.uCallbackMessage = WM_USER_SHELLICON;

    if (!Shell_NotifyIcon(NIM_ADD, &nid))
    {
        Error(L"Shell_NotifyIcon failed!");
        return false;
    }

    return true;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE,
                      _In_ LPWSTR,
                      _In_ int)
{
    if (CreateTrayWindow(hInstance) && CreateTooltipWindow(hInstance))
    {
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }   

    Shell_NotifyIcon(NIM_DELETE, &nid);
    return 0;
}
