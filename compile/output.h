#pragma once

void out_begin();
void out_flush(const char* path);
void out_end();

u64 out_offset();
void out_seek(u64 offset);
void out_seek_end();

void out_write(void* ptr, u32 size);
#define out_write_t(val) (out_write(&val, sizeof(val)))
void out_write_bits(u8 bits, u8 count);
void out_write_u8(u8 val);
void out_write_i8(i8 val);
void out_write_u16(u16 val);
void out_write_i16(i16 val);