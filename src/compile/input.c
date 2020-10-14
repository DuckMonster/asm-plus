#include "input.h"
#include <stdio.h>
#include "log.h"
#include "parse.h"

static char* buffer = NULL;
static char* buffer_ptr = NULL;
static u32 buffer_size;
static const char* buffer_path;

void in_load(const char* path)
{
	timer_push();

	if (buffer != NULL)
		free(buffer);

	FILE* file = fopen(path, "rb");
	if (file == NULL)
		error("Failed to open input file %s", path);

	buffer_path = path;

	fseek(file, 0, SEEK_END);
	buffer_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	buffer = buffer_ptr = malloc(buffer_size);
	fread(buffer, 1, buffer_size, file);
	fclose(file);

	log_writel(LOG_TRIVIAL, "Loaded file '%s', %d bytes (%.2f ms)", path, buffer_size, timer_pop_ms());
}

u32 in_size()
{
	return buffer_size;
}

u32 in_remaining()
{
	return (u32)(buffer_size - (buffer_ptr - buffer));
}

char* in_ptr()
{
	return buffer_ptr;
}

void in_adv(int amount)
{
	buffer_ptr += amount;
	if (buffer_ptr > buffer + buffer_size)
		buffer_ptr = buffer + buffer_size;
}

bool in_eof()
{
	return (buffer_ptr) >= (buffer + buffer_size);
}

i32 in_line_at(const char* ptr)
{
	i32 line;
	i32 col;
	in_line_col_at(ptr, &line, &col);
	return line;
}

i32 in_col_at(const char* ptr)
{
	i32 line;
	i32 col;
	in_line_col_at(ptr, &line, &col);
	return col;
}

void in_line_col_at(const char* ptr, i32* out_line, i32* out_col)
{
	i32 line = 0;
	i32 col = 0;

	char* offset = buffer;
	while(offset != ptr)
	{
		// Carriage return, windows...
		if (*offset == '\r')
		{
			offset++;
			continue;
		}

		col++;
		if (*offset == '\n')
		{
			col = 0;
			line++;
		}

		offset++;
	}

	*out_line = line + 1;
	*out_col = col + 1;
}

const char* in_line(i32 line)
{
	if (line < 0)
		return NULL;

	const char* ptr = buffer;
	while(line > 1 || *ptr=='\r')
	{
		if (ptr >= buffer + buffer_size)
			return NULL;

		if (*ptr == '\n')
			line--;

		ptr++;
	}

	return ptr;
}

const char* in_line_start(const char* ptr)
{
	// Carriage return
	if (*ptr == '\r')
		ptr--;

	if (ptr == buffer)
		return ptr;

	while(ptr-- > buffer)
	{
		if (ptr == buffer || NEWLINE(ptr - 1))
			break;
	}

	return ptr;
}

const char* in_line_end(const char* ptr)
{
	// Carriage return
	if (*ptr == '\r')
		ptr++;

	while(ptr++ < (buffer + buffer_size))
	{
		if (ptr == (buffer + buffer_size) || NEWLINE(ptr))
			break;
	}

	return ptr;
}

u32 in_line_len(const char* ptr)
{
	return (u32)(in_line_end(ptr) - in_line_start(ptr));
}

const char* in_path()
{
	return buffer_path;
}

const char* in_filename()
{
	u32 len = (u32)strlen(buffer_path);
	if (len == 0)
		return buffer_path;

	const char* ptr = buffer_path + len;
	while(--ptr > buffer_path)
	{
		if (*ptr == '\\' || *ptr == '/')
			return ptr + 1;
	}

	return buffer_path;
}