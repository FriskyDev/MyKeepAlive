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

#define STRLEN(str) (int)wcslen(str)
#define BUFSTR(buf) (LPWSTR)&buf

// main.cpp
extern HINSTANCE hInstance;
extern bool paused;

// tray.cpp
extern int TotalTimeRunningM;
extern int DelayRemainingM;
bool CreateTrayWindow();

// preview.cpp
extern bool PreviewShowing;
extern HWND hwndPreview;
bool CreatePreviewWindow();
void ShowPreviewWindow();
void HidePreviewWindow();

// helpers.cpp
void Error(std::wstring msg);
UINT GetDpiForWindow(HWND hwnd);
void WellBehavedTrackPopup(HWND hwnd, HMENU hMenu, POINT pt);
RECT KeepRectInRect(RECT rcStartRect, RECT rc);
POINT KeepPointInRect(POINT pt, RECT rc);
RECT WorkAreaFromPoint(POINT pt);
UINT DpiFromPt(POINT pt);
void DaysMinsSecsFromMinutes(
    _In_ UINT minutes, _Out_ UINT* days,
    _Out_ UINT* hours, _Out_ UINT* minutesRemaining);

