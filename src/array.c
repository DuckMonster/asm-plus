#include "array.h"
#include <math.h>

void _array_reserve(Array* arr, u32 size, u32 new_cap)
{
	if (new_cap <= arr->capacity)
		return;

	void* old_ptr = arr->data;
	arr->data = malloc(size * new_cap);
	arr->capacity = new_cap;
	memzero(arr->data, size * new_cap);

	if (old_ptr)
	{
		memcpy(arr->data, old_ptr, arr->count * size);
		free(old_ptr);
	}
}

void _array_add(Array* arr, u32 size)
{
	if (arr->count == arr->capacity)
	{
		u32 new_cap = 4;
		if (arr->capacity > 0)
		{
			u32 po2 = (u32)log2(arr->capacity);
			new_cap = 1 << (po2 + 1);
		}

		_array_reserve(arr, size, new_cap);
	}

	arr->count++;
}