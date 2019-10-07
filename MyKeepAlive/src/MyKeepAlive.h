#pragma once

#include <windows.h>
#include "Windowsx.h"
#include "Shellapi.h"
#include "ShellScalingAPI.h"
#include <tchar.h>
#include <string>
#include "..\resource.h"

#define STRLEN(str) (int)wcslen(str)
#define BUFSTR(buf) (LPWSTR)&buf

extern HINSTANCE hInstance;
void Error(std::wstring msg);
void Warning(std::wstring msg);

bool IsAlreadyRunning();
bool CreateTrayWindow();
bool CreatePreviewWindow();

void ShowHidePreview(bool show);
bool Paused();
void TogglePaused();

void WellBehavedTrackPopup(HWND hwnd, HMENU hMenu, POINT pt);
RECT KeepRectInRect(RECT rcStartRect, RECT rc);
POINT KeepPointInRect(POINT pt, RECT rc);
RECT WorkAreaFromPoint(POINT pt);
UINT DpiFromPt(POINT pt);
