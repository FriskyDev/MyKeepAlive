
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

HINSTANCE hInstance = nullptr;
CTimer* gTimer = nullptr;

void InjectBogusKeyboardInput()
{
    static INPUT i = { INPUT_KEYBOARD,{ VK_F24, 0, 0, 0, 0 } };
    SendInput(1, &i, sizeof(INPUT));
}

void RightClickMenu(HWND hwnd, POINT pt)
{
    HMENU hMenu = CreatePopupMenu();

    InsertMenu(hMenu, 0xFFFFFFFF,
        MF_BYPOSITION | MF_STRING,
        IDM_EXIT, L"Exit");

    InsertMenu(hMenu, 0xFFFFFFFF,
        MF_BYPOSITION | MF_STRING,
        IDM_TOGGLEPAUSE, L"Pause");

    CheckMenuItem(hMenu, IDM_TOGGLEPAUSE,
        gTimer->Paused() ? MF_CHECKED : MF_UNCHECKED);

    if (!gTimer->Paused())
    {
        WCHAR mnItemBuf[100];
        if (gTimer->Delayed())
        {
            UINT hours, minutes;
            gTimer->HrsMinDelayed(&hours, &minutes);

            swprintf(BUFSTR(mnItemBuf), 100,
                L"Pause in %i hr %i min", hours, minutes);
        }
        else
        {
            swprintf(BUFSTR(mnItemBuf), 100, L"Pause in 5 hours");
        }

        InsertMenu(hMenu, 0xFFFFFFFF,
            MF_BYPOSITION | MF_STRING,
            IDM_TOGGLE5HRDELAY, BUFSTR(mnItemBuf));

        CheckMenuItem(hMenu, IDM_TOGGLE5HRDELAY,
            gTimer->Delayed() ? MF_CHECKED : MF_UNCHECKED);
    }

    WellBehavedTrackPopup(hwnd, hMenu, pt);
}

int APIENTRY wWinMain(HINSTANCE hinst, HINSTANCE, LPWSTR, int)
{
    hInstance = hinst;
    CTimer timer(UpdateIconAndTooltip, InjectBogusKeyboardInput);
    gTimer = &timer;

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
