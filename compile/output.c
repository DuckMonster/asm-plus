#include "output.h"
#include <stdio.h>
FILE* file;

u8 bit_buffer;
u8 bit_offset;
u64 offset;

void out_begin(const char* path)
{
	file = fopen(path, "wb");

	offset = 0;
	bit_buffer = 0;
	bit_offset = 0;
}

u64 out_offset()
{
	return (u64)ftell(file);
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

void out_write_u8(u8 val)
{
	fwrite(&val, 1, 1, file);
}

void out_write_i8(i8 val)
{
	fwrite(&val, 1, 1, file);
}

void out_write_u16(u16 val)
{
	fwrite(&val, 1, 2, file);
}

void out_write_i16(i16 val)
{
	fwrite(&val, 1, 2, file);
}

void out_end()
{
	fclose(file);
}