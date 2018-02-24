
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

#define IDT_TIMER           101
#define IDT_TIMER_LONG      102

const UINT TimerMS = 10000;          // 10 seconds
const UINT LongTimerMS = 60000;      // 1 minute
const UINT DelayTimeoutM = 60 * 5;   // 5 hours
CTimer* gTimer = nullptr;

void CTimer::DaysHrsMinDelayed(UINT* days, UINT* hrs, UINT* min)
{
    DaysMinsSecsFromMinutes(DelayRemainingM, days, hrs, min);
}

void CTimer::DaysHrsMinTotal(UINT* days, UINT* hrs, UINT* min)
{
    DaysMinsSecsFromMinutes(TotalTimeRunningM, days, hrs, min);
}

void CTimer::ToggleDelay()
{
    if (DelayRemainingM < 0)
    {
        DelayRemainingM = DelayTimeoutM;
    }
    else
    {
        DelayRemainingM = -1;
    }

    fnUpdateUI();
}

void CTimer::TogglePaused()
{
    paused = !paused;

    fnUpdateUI();
}

void CTimer::Callback(UINT id)
{
    if (id == IDT_TIMER_LONG) /* fires every minute */
    {
        TotalTimeRunningM++;

        if (DelayRemainingM == 0)
        {
            paused = true;
        }
        else if (DelayRemainingM > 0)
        {
            DelayRemainingM--;
        }
    }
    else /* IDT_TIMER, fires every ten seconds */
    {
        if (!paused)
        {
            fnInject();
        }
    }

    fnUpdateUI();
}

CTimer::CTimer(HWND hwnd, void(*_fnUpdateUI)(), void(*_fnInject)())
    : fnUpdateUI(_fnUpdateUI), fnInject(_fnInject)
{
    SetTimer(hwnd, IDT_TIMER, TimerMS, nullptr);
    SetTimer(hwnd, IDT_TIMER_LONG, LongTimerMS, nullptr);
}
