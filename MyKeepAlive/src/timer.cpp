
#include "stdafx.h"
#include "MyKeepAlive.h"
using namespace std;

//
// Maintain timers, the paused state, and the 'delay' timeout
//  - Every 10 seconds (unless paused), calls InjectBogusKeyboardInput.
//  - When delay is enabled, will auto-pause in 5 hours.
//  - When pause state, or delay time changes, calls UpdateIconAndTooltip.
//

#define IDT_TIMER           101
#define IDT_TIMER_LONG      102

bool paused = startpaused;

const UINT TimerMS = 10000;         // 10 seconds
const UINT LongTimerMS = 60000;     // 1 minute
const UINT DelayTimeoutM = 60 * 5;  // 5 hours
int DelayRemainingM = -1;           // tracks minutes until auto-pause
int TotalTimeRunningM = 0;          // tracks total minutes since launched

bool Paused()
{
    return paused;
}

bool Delayed()
{
    return DelayRemainingM > 0;
}

void HrsMinDelayed(UINT* hrs, UINT* min)
{
    UINT days;
    DaysMinsSecsFromMinutes(DelayRemainingM, &days, hrs, min);
}

void DaysHrsMinTotal(UINT* days, UINT* hrs, UINT* min)
{
    DaysMinsSecsFromMinutes(TotalTimeRunningM, days, hrs, min);
}

void ToggleDelay()
{
    if (DelayRemainingM < 0)
    {
        DelayRemainingM = DelayTimeoutM;
    }
    else
    {
        DelayRemainingM = -1;
    }

    UpdateIconAndTooltip();
}

void TogglePaused()
{
    paused = !paused;

    UpdateIconAndTooltip();
}

void TimerCallback(UINT id)
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

        UpdateIconAndTooltip();
    }
    else /* IDT_TIMER, fires every ten seconds */
    {
        if (!paused)
        {
            InjectBogusKeyboardInput();
        }
    }
}

void CreateTimers(HWND hwnd)
{
    SetTimer(hwnd, IDT_TIMER, TimerMS, nullptr);
    SetTimer(hwnd, IDT_TIMER_LONG, LongTimerMS, nullptr);
}
