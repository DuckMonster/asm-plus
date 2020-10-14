#include "timing.h"
#include <windows.h>

#define MAX_TIMERS 32
static LARGE_INTEGER timer_stack[MAX_TIMERS];
static u32 timer_index = 0;

void timer_push()
{
	QueryPerformanceCounter(timer_stack + timer_index);
	timer_index++;
}

f64 timer_pop_ms()
{
	LARGE_INTEGER freq;
	LARGE_INTEGER now;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&now);

	LARGE_INTEGER timer = timer_stack[--timer_index];
	u64 delta = now.QuadPart - timer.QuadPart;

	return ((f64)delta / freq.QuadPart) * 1000;
}