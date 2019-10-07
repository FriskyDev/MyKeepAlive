
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

HINSTANCE hInstance = nullptr;

void CheckSetForStartup()
{
	// Get image path
	WCHAR buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);

	string::size_type pos = wstring(buffer).find_last_of(L"\\/");
	if (pos == string::npos)
	{
		Error(L"GetModuleFileName failed!");
		return;
	}

	HKEY hkey = NULL;
	PCWSTR keyname = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
	PCWSTR keyval = L"mykeepalive";

	// MessageBox to ask whether to run on startup
	if ((MessageBox(
		NULL,
		L"Set to run at startup?",
		L"KeeyAlive - Ghost input to keep monitors awake.",
		MB_YESNO) == IDYES))
	{
		// Set the on startup path
		RegCreateKey(HKEY_CURRENT_USER, keyname, &hkey);
		RegSetValueEx(hkey, keyval, 0, REG_SZ, (BYTE*)(PWSTR)buffer, (DWORD)(wcslen((PWSTR)buffer) + 1) * 2);
	}
	else
	{
		// Clear the on startup path
		RegDeleteKeyValue(HKEY_CURRENT_USER, keyname, keyval);
	}
}

int APIENTRY wWinMain(HINSTANCE hinst, HINSTANCE, LPWSTR, int)
{
	// Dis-allow multiple instances
	if (IsAlreadyRunning())
	{
		Error(L"KeepAlive already running!");
		return 1;
	}

	// Prompt for run on startup
	CheckSetForStartup();

	// Initialize the windows
	hInstance = hinst;
	if (!CreateTrayWindow() || !CreatePreviewWindow())
	{
		return 2;
	}

	// Message pump
	MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
