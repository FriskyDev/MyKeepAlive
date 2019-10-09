
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

HINSTANCE hInstance = nullptr;
bool paused = false;
bool runonstartup = true;


//
// InjectGhostInput
//
// SendInput (INPUT_MOUSE) with a delta of zero.
// Causes the system to generate mouse input that is effectively ignored.
// ALSO bumps the last user input timer, which some apps use to
// decide to save power by turning things off.
//
// When not paused, this is called on a 10 second timer to keep applications/
// graphics drivers, which use the last user time, from timing out.
//

void InjectGhostInput()
{
    static INPUT i = { INPUT_MOUSE, {} };
    SendInput(1, &i, sizeof(INPUT));
}


//
// Global paused state
//

bool IsPaused()
{
    return paused;
}

void ToggleIsPaused()
{
    // Flip paused state
    paused = !paused;

    // Update tray (icon, tooltip)
    UpdateTrayWindow();

    // Update preview window, if showing
    UpdatePreviewWindow();
}


//
// By default, adds self to the run on startup registry key
// In right-click context menu, option exists to turn off (remove value from regkey)
//

void SetRunOnStartup()
{
    // Get image path
    WCHAR buffer[MAX_PATH] = {};
    PCWSTR imagepath = (PWSTR)buffer;

    if ((GetModuleFileName(NULL, buffer, MAX_PATH) == 0) ||
        (wcslen((PWSTR)buffer) == 0))
    {
        Error(L"GetModuleFileName failed!");
        return;
    }

    PCWSTR keyname = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    PCWSTR keyval = L"mykeepalive";

    if (runonstartup)
    {
        // Set the on startup path
        HKEY hkey = NULL;
        RegCreateKey(HKEY_CURRENT_USER, keyname, &hkey);
        RegSetValueEx(hkey, keyval, 0, REG_SZ, (BYTE*)(PWSTR)buffer, (DWORD)(wcslen((PWSTR)buffer) + 1) * 2);
    }
    else
    {
        // Clear the on startup path
        RegDeleteKeyValue(HKEY_CURRENT_USER, keyname, keyval);
    }
}

bool IsRunOnStartupSet()
{
    return runonstartup;
}

void ToggleRunOnStartup()
{
    runonstartup = !runonstartup;

    SetRunOnStartup();
}


//
// main()
//

int APIENTRY wWinMain(HINSTANCE hinst, HINSTANCE, LPWSTR, int)
{
    // Dis-allow multiple instances
    if (IsAlreadyRunning())
    {
        Error(L"KeepAlive already running!");
        return 1;
    }

    // Set run on startup regkey
    SetRunOnStartup();

    // todo, remember if was last paused and restore state?

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
