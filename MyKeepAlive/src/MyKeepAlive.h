#pragma once

#include <windows.h>
#include "Windowsx.h"
#include "Shellapi.h"
#include "ShellScalingAPI.h"
#include <tchar.h>
#include <string>
#include "..\resource.h"

#define IDM_EXIT            111
#define IDM_TOGGLEPAUSE     112
#define	WM_USER_SHELLICON   WM_USER + 1
#define STRLEN(str) (int)wcslen(str)
#define BUFSTR(buf) (LPWSTR)&buf

extern HINSTANCE hInstance;
void Error(std::wstring msg);

bool CreatePreviewWindow();
void CreateTimers(HWND hwnd);
void InjectBogusInput();

void RightClickMenu(HWND hwndTray, POINT pt);
bool CreateTrayWindow();
void UpdateRunningState();
void ShowHidePreview(bool show);
bool Paused();
void TogglePaused();

void WellBehavedTrackPopup(HWND hwnd, HMENU hMenu, POINT pt);
RECT KeepRectInRect(RECT rcStartRect, RECT rc);
POINT KeepPointInRect(POINT pt, RECT rc);
RECT WorkAreaFromPoint(POINT pt);
UINT DpiFromPt(POINT pt);

