#pragma once
typedef unsigned char u8;
typedef signed char i8;
typedef unsigned short u16;
typedef signed short i16;
typedef unsigned int u32;
typedef signed int i32;
typedef unsigned long long u64;
typedef long long i64;
typedef float f32;
typedef double f64;

typedef unsigned char bool;
enum { false, true };

#include <string.h>
#define memzero(ptr, size) (memset(ptr, 0, size))
#define memzero_t(var) (memzero(&var, sizeof(var)))

inline char* str_add(char* tar, const char* src, u32 size)
{
	memcpy(tar, src, size);
	return tar + size;
}