#pragma once

void out_begin();
void out_flush_file(const char* path);
void out_flush_mem(void** ptr);
void out_end();
char* out_ptr();

u64 out_offset();
void out_seek(u64 offset);
void out_seek_end();

void out_seek_push(u64 offset);
u64 out_seek_pop();

void out_write(const void* ptr, u32 size);
#define out_write_t(val) (out_write(&val, sizeof(val)))
void out_write_bits(u8 bits, u8 count);
void out_write_u8(u8 val);
void out_write_i8(i8 val);
void out_write_u16(u16 val);
void out_write_i16(i16 val);
void out_write_u32(u32 val);
void out_write_i32(i32 val);
void out_write_u64(u64 val);
void out_write_i64(i64 val);
void out_write_str(const char* str);