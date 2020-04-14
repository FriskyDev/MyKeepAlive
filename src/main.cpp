
#include "MyKeepAlive.h"

#include <commctrl.h>
#pragma comment(lib, "Comctl32.lib")

using namespace std;

HINSTANCE hInstance = nullptr;
bool paused = false;
bool runonstartup = true;
bool onschedule = false;
SYSTEMTIME g_timeFrom{};
SYSTEMTIME g_timeTo{};

PCWSTR Reg_MyKeepAlive_Base = L"Software\\MyKeepAlive";
PCWSTR Reg_TimeFrom = L"TimeFrom";
PCWSTR Reg_TimeTo = L"TimeTo";
PCWSTR Reg_KeyName = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
PCWSTR Reg_KeyValue = L"mykeepalive";

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

bool IsScheduled()
{
    return onschedule;
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

void LoadTime(SYSTEMTIME& time, WORD hour = 0, WORD minute = 0, WORD second = 0, WORD millisecond = 0)
{
    GetLocalTime(&time);
    time.wHour = hour;
    time.wMinute = minute;
    time.wSecond = second;
    time.wMilliseconds = millisecond;
}

void LoadSchedule(SYSTEMTIME& timeFrom, SYSTEMTIME& timeTo)
{
    HKEY hkey{};
    DWORD len = 8;
    DWORD type{};
    LoadTime(timeFrom);
    LoadTime(timeTo);
    if (RegOpenKey(HKEY_CURRENT_USER, Reg_MyKeepAlive_Base, &hkey) == ERROR_SUCCESS) {
        auto result = RegQueryValueEx(hkey, Reg_TimeFrom, 0, &type, (LPBYTE)&timeFrom.wHour, &len);
        if (result != ERROR_SUCCESS) {
            auto msg = GetErrorMessage(result);
            OutputDebugString(msg.c_str());
        }
        len = 8;
        result = RegQueryValueEx(hkey, Reg_TimeTo, 0, &type, (LPBYTE)&timeTo.wHour, &len);
        if (result != ERROR_SUCCESS) {
            auto msg = GetErrorMessage(result);
            OutputDebugString(msg.c_str());
        }
        RegCloseKey(hkey);
    }
}

void SaveSchedule(SYSTEMTIME& timeFrom, SYSTEMTIME& timeTo)
{
    HKEY hkey{};
    if (RegOpenKey(HKEY_CURRENT_USER, Reg_MyKeepAlive_Base, &hkey) != ERROR_SUCCESS) {
        // create the key
        RegCreateKeyEx(HKEY_CURRENT_USER, Reg_MyKeepAlive_Base, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hkey, NULL);
    }

    if (hkey != NULL) {
        if (RegSetValueEx(hkey, Reg_TimeFrom, 0, REG_QWORD, (LPCBYTE)&timeFrom.wHour, 8) != ERROR_SUCCESS) {
            auto msg = GetErrorMessage();
            OutputDebugString(msg.c_str());
        }
        if (RegSetValueEx(hkey, Reg_TimeTo, 0, REG_QWORD, (LPCBYTE)&timeTo.wHour, 8) != ERROR_SUCCESS) {
            auto msg = GetErrorMessage();
            OutputDebugString(msg.c_str());
        }
        RegCloseKey(hkey);
    }
}

uint64_t TimeFromDate(const SYSTEMTIME& time)
{
    return ((uint64_t)time.wHour << 48) + ((uint64_t)time.wMinute << 32) + (time.wSecond << 16) + time.wMilliseconds;
}


bool TimeInSchedule(const SYSTEMTIME& time)
{
    auto timeNow = TimeFromDate(time);
    return timeNow >= TimeFromDate(g_timeFrom) && timeNow <= TimeFromDate(g_timeTo);
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

    if (runonstartup)
    {
        // Set the on startup path
        HKEY hkey = NULL;
        if (RegCreateKey(HKEY_CURRENT_USER, Reg_KeyName, &hkey) == ERROR_SUCCESS) {
            RegSetValueEx(hkey, Reg_KeyValue, 0, REG_SZ, (BYTE*)(PWSTR)buffer, (DWORD)(wcslen((PWSTR)buffer) + 1) * 2);
            RegCloseKey(hkey);
        }
    }
    else
    {
        // Clear the on startup path
        RegDeleteKeyValue(HKEY_CURRENT_USER, Reg_KeyName, Reg_KeyValue);
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

void ToggleSchedule()
{
    onschedule = !onschedule;
}

LRESULT CALLBACK ConfigureDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK: {
            // save the new schedule
            DateTime_GetSystemtime(GetDlgItem(hWnd, IDC_TIMEFROM), &g_timeFrom);
            DateTime_GetSystemtime(GetDlgItem(hWnd, IDC_TIMETO), &g_timeTo);
            EndDialog(hWnd, wParam);
            break;
        }
        case IDCANCEL:
            EndDialog(hWnd, wParam);
            return TRUE;
        }
    case WM_INITDIALOG:
        if (GetDlgCtrlID((HWND)wParam) != IDC_TIMEFROM) {
            DateTime_SetSystemtime(GetDlgItem(hWnd, IDC_TIMEFROM), GDT_VALID, &g_timeFrom);
            DateTime_SetSystemtime(GetDlgItem(hWnd, IDC_TIMETO), GDT_VALID, &g_timeTo);
            SetFocus(GetDlgItem(hWnd, IDC_TIMEFROM));
            return FALSE;
        }
    }
    return FALSE;
}

void Configure()
{
    if (DialogBox(nullptr, MAKEINTRESOURCE(IDD_CONFIG), nullptr, (DLGPROC)ConfigureDlgProc) == IDOK)
    {
        // save new configuration
        SaveSchedule(g_timeFrom, g_timeTo);
    }
}


//
// main()
//

int APIENTRY wWinMain(HINSTANCE hinst, HINSTANCE, LPWSTR, int)
{
    InitCommonControls();

    LoadSchedule(g_timeFrom, g_timeTo);

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
