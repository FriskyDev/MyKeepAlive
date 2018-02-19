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
#define IDM_PAUSE_IN_5HRS   113
#define	WM_USER_SHELLICON   WM_USER + 1

extern NOTIFYICONDATA nid;
extern HWND hwndTooltip;

void Error(std::wstring msg);
UINT GetDpiForWindow(HWND hwnd);
void WellBehavedTrackPopup(HWND hwnd, HMENU hMenu, POINT pt);
RECT KeepRectInRect(RECT rcStartRect, RECT rc);
POINT KeepPointInRect(POINT pt, RECT rc);
RECT WorkAreaFromPoint(POINT pt);
UINT DpiFromPt(POINT pt);

LRESULT CALLBACK TrayWindowWndProc(
    HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

extern bool TooltipShowing;
RECT ShowHoverTooltip(POINT pt);
void HideHoverTooltip();
LRESULT CALLBACK TooltipWndProc(
    HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

