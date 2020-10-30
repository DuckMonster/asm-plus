#pragma once

enum
{
	MACHINE_AMD64 = 0x8664,
};

enum
{
	SCT_CODE = 0x20,
	SCT_INIT_DATA = 0x40,
	SCT_UNINIT_DATA = 0x80,

	SCT_EXEC_CODE = 0x20000000,
	SCT_READ = 0x40000000,
	SCT_WRITE = 0x80000000,
};

typedef struct
{
	u16 machine;
	u16 section_num;

	u32 timestamp;

	u32 symbol_ptr;
	u32 symbol_num;
	u16 ohdr_size;

	u16 flags;
} Coff_Header;

typedef struct
{
	char name[8];
	u32 vsize;
	u32 vaddr;

	u32 size;
	u32 data_ptr;

	u32 reloc_ptr;
	u32 linenmbr_ptr;
	u16 reloc_num;
	u16 linenmbr_num;

	u32 flags;
} Coff_Section_Header;

void coff_write(const char* path);