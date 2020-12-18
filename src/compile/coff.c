#include "coff.h"
#include "output.h"
#include "log.h"
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

u32 ceil_4bit(u32 value)
{
	// Laying on 4bit boundary
	if ((value & 0xF) == 0)
		return value;

	return ((value >> 4) << 4) + 0x10;
}

void coff_write(const char* path, Compile* compile)
{
	timer_push();
	log_writel(LOG_MEDIUM, "== Writing COFF ==");

	out_begin();

	// Header
	out_debug_begin();
	log_writel(LOG_DEV, "COFF header");

	Coff_Header hdr;
	memzero(&hdr, sizeof(hdr));

	hdr.machine = MACHINE_AMD64;
	hdr.section_num = compile->sections.count;

	out_write_t(hdr);
	out_debug_end();

	String_List str_list;
	memzero_t(str_list);

	// Write section headers
	for(u32 section_idx = 0; section_idx < compile->sections.count; ++section_idx)
	{
		Section section = compile->sections.data[section_idx];
		Coff_Section c_section;
		memzero_t(c_section);

		memcpy(c_section.name, section.token.ptr, section.token.len);
		c_section.size = section.data_size;
		c_section.flags = SCT_EXEC_CODE;
		c_section.reloc_num = section.relocations.count;

		log_writel(LOG_DEV, "SECTION '%s'", c_section.name);
		log_writel(LOG_DEV, "\tSize:  %d", c_section.size);
		log_writel(LOG_DEV, "\tFlags: %x", c_section.flags);

		out_debug_begin();
		out_write_t(c_section);
		out_debug_end();
	}

	u32 end_ptr = ceil_4bit(out_offset());

	// Write section contents
	out_seek(sizeof(Coff_Header));
	for(u32 section_idx = 0; section_idx < compile->sections.count; ++section_idx)
	{
		Section section = compile->sections.data[section_idx];
		Coff_Section c_section = *(Coff_Section*)out_ptr();
		c_section.data_ptr = end_ptr;

		// Write data
		log_writel(LOG_DEV, "'%s' data", c_section.name);
		out_seek_push(end_ptr);

		out_debug_begin();
		out_write(section.data, section.data_size);
		out_debug_end();

		// Write relocations
		out_seek(ceil_4bit(out_offset()));
		c_section.reloc_ptr = out_offset();

		for(u32 reloc_idx = 0; reloc_idx < section.relocations.count; ++reloc_idx)
		{
			Relocation reloc = section.relocations.data[reloc_idx];
			Coff_Relocation c_reloc;
			c_reloc.addr = reloc.ptr;
			c_reloc.sym_index = reloc.sym_index;
			switch(reloc.type)
			{
				case RELOC_MEM: c_reloc.type = RELOC_ABS64; break;
				case RELOC_FUNC: c_reloc.type = RELOC_REL32; break;
			}

			log_writel(LOG_DEV, "Reloc: addr %08X, symbol %d", c_reloc.addr, c_reloc.sym_index);
			out_write_t(c_reloc);
		}

		end_ptr = ceil_4bit(out_seek_pop());

		out_write_t(c_section);
	}

	// Update symbol data in file header
	out_seek(0);
	hdr.symbol_num = compile->symbols.count;
	hdr.symbol_ptr = end_ptr;
	out_write_t(hdr);

	out_seek(end_ptr);

	// Write all symbols
	for(u32 sym_idx = 0; sym_idx < compile->symbols.count; ++sym_idx)
	{
		Symbol symbol = compile->symbols.data[sym_idx];

		Coff_Symbol c_symbol;
		memzero_t(c_symbol);

		if (symbol.token.len > 8)
			c_symbol.longname.offset = get_or_add_string(&str_list, symbol.token.ptr, symbol.token.len) + 4;
		else
			memcpy(c_symbol.shrtname, symbol.token.ptr, symbol.token.len);

		c_symbol.ptr = symbol.ptr;
		if (symbol.type == SYM_IMPORT)
			c_symbol.section = 0;
		else
			c_symbol.section = symbol.section + 1; // Symbol-index are 1-based

		c_symbol.type = SYMTYPE_FUNC;
		c_symbol.storage_cls = SYMCLS_EXTERNAL;

		out_write_t(c_symbol);
	}

	// Write string table
	out_write_u32(str_list.size + 4);
	out_write(str_list.data, str_list.size);

	log_writel(LOG_MEDIUM, "== COFF complete (%.2f ms) ==\n", timer_pop_ms());

	out_flush_file(path);
}