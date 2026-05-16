#include "platform.h"
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

void platform_delay_ms(int ms)
{
#ifndef _WIN32
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
#else
    Sleep(ms);
#endif
}
