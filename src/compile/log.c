#include "log.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>
#include "input.h"
#include "parse.h"

u32 log_lvl = LOG_DEV;
u32 error_count = 0;
u32 warning_count = 0;

void log_writel(u32 level, const char* msg, ...)
{
	if (level < log_lvl)
		return;

	va_list vl;
	va_start(vl, msg);
	vprintf(msg, vl);
	va_end(vl);

	printf("\n");
}

void log_write(u32 level, const char* msg, ...)
{
	if (level < log_lvl)
		return;

	va_list vl;
	va_start(vl, msg);
	vprintf(msg, vl);
	va_end(vl);
}

void error(const char* msg, ...)
{
	va_list vl;
	va_start(vl, msg);
	vprintf(msg, vl);
	va_end(vl);

	printf("\n");
	system("pause");
	exit(1);
}

void print_code_preview(const char* ptr, u32 len)
{
	/* Print preview line */
	const char* line_start = in_line_start(ptr);
	u32 line_len = in_line_len(ptr);
	printf("%.*s\n", line_len, line_start);

	/* Print column error and squiggles */
	// Print whitespace up until column
	const char* wht_ptr = line_start;
	while(wht_ptr < ptr)
	{
		if (WHITESPACE(wht_ptr))
			printf("%c", *wht_ptr);
		else
			printf(" ");

		wht_ptr++;
	}

	// Arrow
	printf("^");

	// Squiggly
	for(u32 i=0; i<len - 1; ++i)
		printf("~");

	printf("\n");
}

void error_at(Token token, const char* msg, ...)
{
	u32 line, col;
	in_line_col_at(token.ptr, &line, &col);

	// Print error message
	va_list vl;
	va_start(vl, msg);
	printf("Error (%d:%d) ", line, col);
	vprintf(msg, vl);
	va_end(vl);

	printf("\n");
	print_code_preview(token.ptr, token.len);
	printf("\n");

	error_count++;
}

void warning_at(Token token, const char* msg, ...)
{
	u32 line, col;
	in_line_col_at(token.ptr, &line, &col);

	// Print warning message
	va_list vl;
	va_start(vl, msg);
	printf("Warning (%d:%d) ", line, col);
	vprintf(msg, vl);
	va_end(vl);

	printf("\n");
	print_code_preview(token.ptr, token.len);
	printf("\n");

	warning_count++;
}