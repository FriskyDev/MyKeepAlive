
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;
HINSTANCE hInstance = nullptr;

bool paused = false;
const UINT TimerMS = 10000; // 10 seconds

void InjectBogusInput()
{
	static INPUT i = { INPUT_MOUSE,{} };
	SendInput(1, &i, sizeof(INPUT));
}

bool Paused()
{
	return paused;
}

void TogglePaused()
{
	paused = !paused;

	UpdateRunningState();
}

void TimerCallback(HWND, UINT, UINT_PTR, DWORD)
{
	if (!Paused())
	{
		InjectBogusInput();
	}
}

void CreateTimers(HWND hwnd)
{
#define IDT_TIMER 101
	SetTimer(hwnd, IDT_TIMER, TimerMS, TimerCallback);
}

void SetForStartup()
{
	// todo, some ui to give choice if set on startup?
	// maybe check and message box prompt?

	WCHAR buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	string::size_type pos = wstring(buffer).find_last_of(L"\\/");
	if (pos != string::npos)
	{
		HKEY hkey = NULL;
		RegCreateKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", &hkey);
		RegSetValueEx(hkey, L"keepalive", 0, REG_SZ, (BYTE*)wstring(buffer).c_str(), (DWORD)(wcslen(wstring(buffer).c_str()) + 1) * 2);
	}
}

int APIENTRY wWinMain(HINSTANCE hinst, HINSTANCE, LPWSTR, int)
{
	SetForStartup();

	hInstance = hinst;
    if (CreateTrayWindow() && CreatePreviewWindow())
    {
        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }   

    return 0;
}
