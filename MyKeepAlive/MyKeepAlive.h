#pragma once

#include <windows.h>
#include "Windowsx.h"
#include "Shellapi.h"
#include "ShellScalingAPI.h"
#include <tchar.h>
#include <string>
#include "resource.h"

#define IDM_EXIT            111
#define IDM_TOGGLEPAUSE     112
#define IDM_TOGGLE5HRDELAY  113
#define	WM_USER_SHELLICON   WM_USER + 1

#define STRLEN(str) (int)wcslen(str)
#define BUFSTR(buf) (LPWSTR)&buf

// main.cpp
extern HINSTANCE hInstance;
void InjectBogusKeyboardInput();
void RightClickMenu(HWND hwndTray, POINT pt);

// tray.cpp
bool CreateTrayWindow();
void UpdateIconAndTooltip();

// preview.cpp
bool CreatePreviewWindow();
void ShowHidePreview(bool show);

// timer.cpp
void CreateTimers(HWND hwnd);
bool Delayed();
bool Paused();

void Callback(UINT id);
void ToggleDelay();
void TogglePaused();
void HrsMinDelayed(UINT* hrs, UINT* min);
void DaysHrsMinTotal(UINT* days, UINT* hrs, UINT* min);

// helpers.cpp
void Error(std::wstring msg);
void WellBehavedTrackPopup(HWND hwnd, HMENU hMenu, POINT pt);
RECT KeepRectInRect(RECT rcStartRect, RECT rc);
POINT KeepPointInRect(POINT pt, RECT rc);
RECT WorkAreaFromPoint(POINT pt);
UINT DpiFromPt(POINT pt);
void DaysMinsSecsFromMinutes(
    _In_ UINT minutes, _Out_ UINT* days,
    _Out_ UINT* hours, _Out_ UINT* minutesRemaining);

