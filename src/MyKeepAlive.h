#pragma once

#include <string>

#include <windows.h>
#include <Windowsx.h>
#include <Shellapi.h>
#include <ShellScalingAPI.h>
#include <tchar.h>
#include "..\ico\Resource.h"

extern HINSTANCE hInstance;
void Error(const std::wstring& msg);
void Warning(const std::wstring& msg);

bool IsAlreadyRunning();
void InjectGhostInput();
bool IsPaused();
bool IsScheduled();
void ToggleIsPaused();
bool IsRunOnStartupSet();
void ToggleRunOnStartup();
void ToggleSchedule();
void Configure();

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

std::wstring GetErrorMessage(DWORD dwError = 0);

bool TimeInSchedule(const SYSTEMTIME& time);