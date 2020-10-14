#include "output.h"
#include <stdio.h>
#include <math.h>
#include "log.h"

static char* buffer;
static u64 buffer_offset;
static u64 buffer_capacity;
static u64 file_size;

static u8 bit_buffer;
static u8 bit_offset;

void out_begin()
{
	buffer = NULL;
	file_size = buffer_offset = buffer_capacity = 0;

	bit_buffer = 0;
	bit_offset = 0;
}

void out_flush(const char* path)
{
	timer_push();
	FILE* file = fopen(path, "wb");

	if (buffer != NULL)
		fwrite(buffer, 1, file_size, file);

	fclose(file);
	log_writel(LOG_MEDIUM, "Wrote file '%s', %d bytes (%.2f ms)", path, file_size, timer_pop_ms());
}

void out_end()
{
	if (buffer)
		free(buffer);

	buffer = NULL;
}

void grow_buffer(u64 target_cap)
{
	if (target_cap <= buffer_capacity)
		return;

	u64 po2 = (u32)log2((double)target_cap) + 1;
	buffer_capacity = (u64)1 << po2;

	char* new_buffer = malloc(buffer_capacity);
	if (buffer != NULL)
	{
		memcpy(new_buffer, buffer, file_size);
		free(buffer);
	}

	buffer = new_buffer;
}

u64 out_offset()
{
	return buffer_offset;
}

void out_seek(u64 offset)
{
	grow_buffer(offset);
	buffer_offset = offset;
}

void out_seek_end()
{
	buffer_offset = file_size;
}

void out_write(void* ptr, u32 size)
{
	grow_buffer(buffer_offset + size);
	memcpy(buffer + buffer_offset, ptr, size);

	buffer_offset += size;
	if (file_size < buffer_offset)
		file_size = buffer_offset;
}

void out_write_bits(u8 bits, u8 count)
{
	while(count--)
	{
		bit_buffer = bit_buffer << 1;
		bit_buffer |= ((bits >> count) & 1);

		bit_offset++;
		if (bit_offset == 8)
		{
			out_write_u8(bit_buffer);
			bit_offset = 0;
		}
	}
}

void out_write_u8(u8 val) { out_write_t(val); }
void out_write_i8(i8 val) { out_write_t(val); }
void out_write_u16(u16 val) { out_write_t(val); }
void out_write_i16(i16 val) { out_write_t(val); }