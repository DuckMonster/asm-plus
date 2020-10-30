#pragma once
#pragma pack(push, 2)

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

enum
{
	RELOC_REL32 = 0x04,
};

enum
{
	SYMTYPE_NULL = 0x0000,
	SYMTYPE_PTR = 0x0100,
	SYMTYPE_FUNC = 0x0200,
	SYMTYPE_ARRAY = 0x0300,
};

enum
{
	SYMCLS_NULL = 0x0,
	SYMCLS_EXTERNAL = 0x2,
	SYMCLS_STATIC = 0x3,
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
} Coff_Section;

typedef struct
{
	u32 addr;
	u32 sym_index;
	u16 type;
} Coff_Relocation;

typedef struct
{
	union
	{
		char shrtname[8];
		struct
		{
			u32 zeroes;
			u32 offset;
		} longname;
	};

	u32 ptr;
	u16 section;

	u16 type;
	u8 storage_cls;

	u8 auxsymbol_num;
} Coff_Symbol;

void coff_write(const char* path);

#pragma pack(pop)