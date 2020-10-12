#include "log.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include "input.h"
#include "parse.h"

u32 log_lvl = LOG_TRIVIAL;

void log_write(u32 level, const char* msg, ...)
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

void error_at(const char* ptr, u32 len, const char* msg, ...)
{
	u32 line, col;
	in_line_col_at(ptr, &line, &col);

	// Print whole line
	const char* line_start = in_line_start(ptr);
	u32 line_len = in_line_len(ptr);
	printf("%.*s\n", line_len, line_start);

	// Print whitespace up-until error
	const char* wht_ptr = line_start;
	while(wht_ptr < ptr)
	{
		if (WHITESPACE(wht_ptr))
			printf("%c", *wht_ptr);
		else
			printf(" ");

		wht_ptr++;
	}
	printf("^");
	for(u32 i=0; i<len - 1; ++i)
		printf("~");
	printf("\n");

	va_list vl;
	va_start(vl, msg);
	printf("Error (%d:%d) ", line, col);
	vprintf(msg, vl);
	va_end(vl);
}