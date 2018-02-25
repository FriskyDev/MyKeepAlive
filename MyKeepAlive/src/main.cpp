
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

//
// Creates two windows, one that registers as a tray window
// with the shell, the other a small popup window shown when
// the user clicks on the tray icon.
//
// A timer is run by the tray window, which will call
// InjectBogusKeyboardInput periodically to simulate a keyboard
// press.
//

HINSTANCE hInstance = nullptr;

void InjectBogusKeyboardInput()
{
    static INPUT i = { INPUT_KEYBOARD,{ VK_F24, 0, 0, 0, 0 } };
    SendInput(1, &i, sizeof(INPUT));
}

int APIENTRY wWinMain(HINSTANCE hinst, HINSTANCE, LPWSTR, int)
{
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
