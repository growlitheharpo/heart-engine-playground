#include "heart/sleep.h"

#include "priv/SlimWin32.h"

void HeartYield()
{
	Sleep(0);
}

void HeartSleep(uint32_t milliseconds)
{
	Sleep(milliseconds);
}
