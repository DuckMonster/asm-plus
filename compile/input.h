#pragma once

void in_load(const char* path);
u32 in_size();
u32 in_remaining();
char* in_ptr();
void in_adv(int amount);
bool in_eof();

i32 in_line_at(const char* ptr);
i32 in_col_at(const char* ptr);
void in_line_col_at(const char* ptr, i32* out_line, i32* out_col);
const char* in_line(i32 line);
const char* in_line_start(const char* ptr);
const char* in_line_end(const char* ptr);
u32 in_line_len(const char* ptr);

const char* in_path();