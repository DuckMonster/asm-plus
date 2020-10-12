#include "input.h"
#include "log.h"
#include <stdlib.h>
#include <stdio.h>

char* buffer = NULL;
char* buffer_ptr = NULL;
u32 buffer_size;

void in_load(const char* path)
{
	if (buffer != NULL)
		free(buffer);

	FILE* file = fopen(path, "rb");
	if (file == NULL)
		error("Failed to open input file %s", path);

	fseek(file, 0, SEEK_END);
	buffer_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	buffer = buffer_ptr = malloc(buffer_size);
	fread(buffer, 1, buffer_size, file);

	log(LOG_TRIVIAL, "Loaded file '%s', %d bytes", path, buffer_size);
	fclose(file);
}

u32 in_size()
{
	return buffer_size;
}

u32 in_remaining()
{
	return buffer_size - (buffer_ptr - buffer);
}

char* in_ptr()
{
	return buffer_ptr;
}