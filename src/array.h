#pragma once

typedef struct
{
	u8* data;
	u32 count;
	u32 capacity;
} Array;

#define ARRAY_DEF(name, type) typedef struct name ## _C { type* data; u32 count; u32 capacity; } name;

void _array_reserve(Array* arr, u32 size, u32 new_cap);
void _array_add(Array* arr, u32 size);
#define array_add(arr, item) (_array_add((Array*)&arr, sizeof(item)), arr.data[arr.count - 1] = item, &arr.data[arr.count - 1])
#define array_add_zero(arr) (_array_add((Array*)&arr, sizeof(*arr.data)), &arr.data[arr.count - 1])
#define array_foreach(arr) for(auto* it = arr.data; it && it < arr.data + arr.count; ++it)