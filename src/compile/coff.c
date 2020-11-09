#include "coff.h"
#include "output.h"
#include <stdio.h>

typedef struct
{
	char* data;
	u32 size;
	u32 count;
} String_List;

u32 get_or_add_string(String_List* list, const char* str, u32 len)
{
	// See if it exists
	if (list->data)
	{
		u8* ptr = list->data;
		for(u32 i=0; i<list->count; ++i)
		{
			u32 ptr_len = (u32)strlen(ptr);
			if (len == ptr_len && strncmp(ptr, str, len) == 0)
				return (ptr - list->data);

			ptr += (ptr_len + 1);
		}
	}

	// Otherwise, add it
	char* old_data = list->data;
	list->data = malloc(list->size + len + 1);

	// Copy old data..
	if (old_data)
		memcpy(list->data, old_data, list->size);

	u32 new_offset = list->size;
	memcpy(list->data + new_offset, str, len);

	// Zero terminated
	list->data[new_offset + len] = 0;
	list->size += (len + 1);

	return new_offset;
}


void coff_write(const char* path, Compile_Manifest* compile)
{
	out_begin();

	// Header
	Coff_Header hdr;
	memzero(&hdr, sizeof(hdr));

	hdr.machine = MACHINE_AMD64;
	hdr.section_num = compile->sections.count;
	hdr.symbol_ptr = 0x90;
	hdr.symbol_num = 0;

	out_write_t(hdr);

	String_List str_list;
	memzero_t(str_list);

	u32 symbol_num = 0;
	u64 symbol_ptr = 0x90;

	// Write each section
	for(u32 section_idx = 0; section_idx < compile->sections.count; ++section_idx)
	{
		Section section = compile->sections.data[section_idx];
		Coff_Section c_section;
		memzero_t(c_section);

		strcpy(c_section.name, section.name);
		c_section.size = section.data_size;
		c_section.data_ptr = 0x60;
		c_section.flags = SCT_EXEC_CODE;
		c_section.reloc_num = section.relocations.count;
		c_section.reloc_ptr = 0x80;

		out_write_t(c_section);

		// Write code
		out_seek_push(0x60);
		out_write(section.data, section.data_size);
		out_seek_pop();

		// Write redirects
		out_seek_push(0x80);
		for(u32 relocate_idx = 0; relocate_idx < section.relocations.count; ++relocate_idx)
		{
			Relocation relocation = section.relocations.data[relocate_idx];
			Coff_Relocation c_relocation;
			memzero_t(c_relocation);

			c_relocation.addr = relocation.ptr;
			c_relocation.sym_index = relocation.sym_index;
			c_relocation.type = RELOC_REL32;
			out_write_t(c_relocation);
		}
		out_seek_pop();

		// Write section symbols
		out_seek_push(symbol_ptr);

		for(u32 symbol_idx = 0; symbol_idx < section.symbols.count; ++symbol_idx)
		{
			Symbol symbol = section.symbols.data[symbol_idx];
			Coff_Symbol c_symbol;
			memzero_t(c_symbol);

			// Shortname representation
			if (symbol.name_len <= 8)
			{
				memcpy(c_symbol.shrtname, symbol.name, symbol.name_len);
			}
			// Otherwise, add/get name from the stringtable
			else
			{
				u32 offset = get_or_add_string(&str_list, symbol.name, symbol.name_len);
				c_symbol.longname.zeroes = 0;
				c_symbol.longname.offset = offset + 4; // +4 of string-table size 
			}

			c_symbol.type = SYMTYPE_FUNC;
			if (symbol.type == SYM_IMPORT)
			{
				c_symbol.section = 0;
				c_symbol.ptr = 0;
			}
			else
			{
				c_symbol.section = section_idx + 1;
				c_symbol.ptr = symbol.ptr;
			}

			if (symbol.type == SYM_LOCAL)
				c_symbol.storage_cls = SYMCLS_STATIC;
			else
				c_symbol.storage_cls = SYMCLS_EXTERNAL;

			out_write_t(c_symbol);
			symbol_num++;
		}

		symbol_ptr = out_seek_pop();
	}

	// Update num symbols
	out_seek(0);
	hdr.symbol_num = symbol_num;
	out_write_t(hdr);

	// Write string table
	out_seek_end();
	out_write_u32(str_list.size + 4);
	out_write(str_list.data, str_list.size);

	out_flush(path);
}