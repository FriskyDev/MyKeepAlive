#pragma once

#include <windows.h>
#include "Windowsx.h"
#include "Shellapi.h""
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
void WellBehavedTrackPopup(HWND hwnd, HMENU hMenu, POINT pt);
RECT rcCursorPosClippedToWorkArea(int cx, int cy);
POINT ptCursorPosClippedToWorkArea();

LRESULT CALLBACK TrayWindowWndProc(
    HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TooltipWndProc(
    HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
