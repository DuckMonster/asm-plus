#pragma once
#define MAX_REG 0x100
#define MAX_INST 0x100

#define MAX_SYMBOL 1024
#define MAX_SYMBOL_REF 1024

#include "parse.h"

enum
{
	SYM_LOCAL,
	SYM_EXPORT,
	SYM_IMPORT,
};

typedef struct
{
	const char* name;
	u32 name_len;
	u32 ptr;

	u8 type;
} Symbol;
ARRAY_DEF(Symbol_Array, Symbol);

typedef struct
{
	u32 sym_index;
	u32 ptr;
	u32 size;
} Relocation;
ARRAY_DEF(Relocation_Array, Relocation);

typedef struct
{
	const char* name;
	u8* data;
	u32 data_size;

	Symbol_Array symbols;
	Relocation_Array relocations;
} Section;
ARRAY_DEF(Section_Array, Section);

typedef struct
{
	Section_Array sections;
} Compile_Manifest;

void compile_file(const char* path, Compile_Manifest* out_manifest);
