#pragma once

#include <windows.h>
#include "Windowsx.h"
#include "Shellapi.h"
#include "ShellScalingAPI.h"
#include <tchar.h>
#include <string>
#include "..\ico\resource.h"

extern HINSTANCE hInstance;
void Error(std::wstring msg);
void Warning(std::wstring msg);

bool IsAlreadyRunning();
void InjectGhostInput();
bool IsPaused();
void ToggleIsPaused();
bool IsRunOnStartupSet();
void ToggleRunOnStartup();

bool CreateTrayWindow();
void UpdateTrayWindow();

bool CreatePreviewWindow();
void UpdatePreviewWindow();
void ShowHidePreviewWindow(bool show);

void WellBehavedTrackPopup(HWND hwnd, HMENU hMenu, POINT pt);
RECT KeepRectInRect(RECT rcStartRect, RECT rc);
POINT KeepPointInRect(POINT pt, RECT rc);
RECT WorkAreaFromPoint(POINT pt);
UINT DpiFromPt(POINT pt);
void ReCreateFont(HFONT* phFont, UINT size, LPCWSTR fontName);
