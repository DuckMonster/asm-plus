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

u64 seek_stack[32];
u32 seek_stack_idx = 0;

static u32 debug_offset;

void out_begin()
{
	buffer = NULL;
	file_size = buffer_offset = buffer_capacity = 0;

	bit_buffer = 0;
	bit_offset = 0;
}

void out_flush_file(const char* path)
{
	timer_push();
	FILE* file = fopen(path, "wb");
	if (!file)
	{
		error("Failed to flush file '%s'", path);
		return;
	}

	if (buffer != NULL)
		fwrite(buffer, 1, file_size, file);

	fclose(file);
	log_writel(LOG_MEDIUM, "Wrote file '%s', %d bytes (%.2f ms)", path, file_size, timer_pop_ms());
}

void out_flush_mem(void** ptr)
{
	*ptr = malloc(file_size);
	memcpy(*ptr, buffer, file_size);
}

void out_end()
{
	if (buffer)
		free(buffer);

	buffer = NULL;
}

char* out_ptr()
{
	return buffer + buffer_offset;
}

void grow_buffer(u64 target_cap)
{
	if (target_cap <= buffer_capacity)
		return;

	u64 po2 = (u32)log2((double)target_cap) + 1;
	buffer_capacity = (u64)1 << po2;

	if (buffer)
		buffer = realloc(buffer, buffer_capacity);
	else
		buffer = malloc(buffer_capacity);

	memzero(buffer + file_size, buffer_capacity - file_size);
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

void out_seek_push(u64 offset)
{
	seek_stack[seek_stack_idx++] = buffer_offset;
	buffer_offset = offset;

	grow_buffer(offset);
}

u64 out_seek_pop()
{
	if (seek_stack_idx == 0)
		error("Seek stack underflow");

	u64 cur_offset = buffer_offset;
	buffer_offset = seek_stack[--seek_stack_idx];

	return cur_offset;
}

void out_write(const void* ptr, u32 size)
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
void out_write_u32(u32 val) { out_write_t(val); }
void out_write_i32(i32 val) { out_write_t(val); }
void out_write_u64(u64 val) { out_write_t(val); }
void out_write_i64(i64 val) { out_write_t(val); }
void out_write_str(const char* str) { out_write(str, (u32)strlen(str) + 1); }

void out_debug_begin()
{
	debug_offset = buffer_offset;
}

void out_debug_end()
{
	const u8* ptr = &buffer[debug_offset];
	u32 debug_size = buffer_offset - debug_offset;

	// Devlog
	for(u32 i=0; i<debug_size; ++i)
	{
		log_write(LOG_DEV, " %02X", ptr[i]);
		if (i % 16 == 15)
			log_writel(LOG_DEV, "");
	}

	log_writel(LOG_DEV, "");
}