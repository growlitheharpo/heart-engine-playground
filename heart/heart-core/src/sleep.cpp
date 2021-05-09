#include <heart/sleep.h>

#define NOMINMAX 1
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

void HeartYield()
{
	Sleep(0);
}

void HeartSleep(uint32_t milliseconds)
{
	Sleep(milliseconds);
}
