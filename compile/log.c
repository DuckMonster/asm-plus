#include "log.h"
#include <stdarg.h>
#include <stdio.h>

u32 log_lvl = LOG_TRIVIAL;

void log(u32 level, const char* msg, ...)
{
	if (level < log_lvl)
		return;

	va_list vl;
	va_start(vl, msg);
	vprintf(msg, vl);
	va_end(vl);

	printf("\n");
}

void error(const char* msg, ...)
{
	va_list vl;
	va_start(vl, msg);
	vprintf(msg, vl);
	va_end(vl);

	printf("\n");
	exit(1);
}