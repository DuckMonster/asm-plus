#pragma once

void in_load(const char* path);
u32 in_size();
u32 in_remaining();
char* in_ptr();
void in_adv(int amount);
bool in_eof();