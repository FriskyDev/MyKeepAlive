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

extern HINSTANCE hInstance;

// tray.cpp
bool CreateTrayWindow();

// preview.cpp
extern bool PreviewShowing;
bool CreatePreviewWindow();
void ShowPreviewWindow();
void HidePreviewWindow();

// timer.cpp
class CTimer
{
    bool paused = true;
    int TotalTimeRunningM = 0;
    int DelayRemainingM = 0;
    void(*fnUpdateUI)();
    void(*fnInject)();

public:
    CTimer(HWND hwnd, void(*_fnUpdateUI)(), void(*_fnInject)());
    void Callback(UINT id);
    void ToggleDelay();
    void TogglePaused();

    bool Paused() { return paused; }
    bool Delayed() { return DelayRemainingM <= 0; }
    void DaysHrsMinDelayed(UINT* days, UINT* hrs, UINT* min);
    void DaysHrsMinTotal(UINT* days, UINT* hrs, UINT* min);
};
extern CTimer* gTimer;

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

