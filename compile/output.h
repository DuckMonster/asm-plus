#pragma once

void out_begin(const char* path);
u64 out_offset();
void out_write_bits(u8 bits, u8 count);
void out_write_u8(u8 val);
void out_write_i8(i8 val);
void out_write_u16(u16 val);
void out_write_i16(i16 val);
void out_end();