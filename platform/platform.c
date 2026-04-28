#include "platform.h"

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

void platform_delay_ms(int ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}
