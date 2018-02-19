#pragma once

#include <windows.h>
#include "Windowsx.h"
#include "Shellapi.h"
#include "ShellScalingAPI.h"
#include <tchar.h>
#include <string>
#include "resource.h"

#define IDT_TIMER           101
#define IDT_TIMER_LONG      102
#define IDM_EXIT            111
#define IDM_TOGGLEPAUSE     112
#define IDM_TOGGLE5HRDELAY  113
#define	WM_USER_SHELLICON   WM_USER + 1

// main.cpp
extern HINSTANCE hInstance;
extern bool paused;

// tray.cpp
extern int MinToAutoPause;
bool CreateTrayWindow();

// preview.cpp
extern bool PreviewShowing;
bool CreatePreviewWindow();
RECT ShowPreviewWindow(POINT pt);
void HidePreviewWindow();

// helpers.cpp
void Error(std::wstring msg);
UINT GetDpiForWindow(HWND hwnd);
void WellBehavedTrackPopup(HWND hwnd, HMENU hMenu, POINT pt);
RECT KeepRectInRect(RECT rcStartRect, RECT rc);
POINT KeepPointInRect(POINT pt, RECT rc);
RECT WorkAreaFromPoint(POINT pt);
UINT DpiFromPt(POINT pt);
