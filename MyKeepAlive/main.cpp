
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

HINSTANCE hInstance = nullptr;
bool paused = true; // start in the in-active state

int APIENTRY wWinMain(_In_ HINSTANCE _hInstance,
                      _In_opt_ HINSTANCE,
                      _In_ LPWSTR,
                      _In_ int)
{
    hInstance = _hInstance;
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
