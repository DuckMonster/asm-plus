#pragma once
typedef unsigned char u8;
typedef signed char i8;
typedef unsigned short u16;
typedef signed short i16;
typedef unsigned int u32;
typedef signed int i32;

typedef unsigned char bool;
enum { false, true };

#include <string.h>
#define memzero(ptr, size) memset(ptr, 0, size)