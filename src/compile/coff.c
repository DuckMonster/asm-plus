#include "coff.h"
#include "output.h"

void coff_write(const char* path)
{
	out_begin();

	// FILE HEADER
	Coff_Header hdr;
	memzero(&hdr, sizeof(hdr));

	hdr.machine = MACHINE_AMD64;
	hdr.section_num = 1;
	hdr.symbol_ptr = 0x80;
	hdr.symbol_num = 2;

	out_write_t(hdr);

	// CODE SECTION
	Coff_Section sect;
	memzero(&sect, sizeof(sect));

	strcpy(sect.name, "code");
	sect.size = 0x10;
	sect.data_ptr = 0x60;
	sect.flags = SCT_EXEC_CODE;
	sect.reloc_ptr = 0x70;
	sect.reloc_num = 1;

	out_write_t(sect);

	out_seek(0x60);
	out_write_u8(0x48);
	out_write_u8(0x33);

	out_write_bits(0b11, 2);
	out_write_bits(0b000, 3);
	out_write_bits(0b011, 3);

	out_write_u8(0xE8);
	out_write_u32(0);

	// RELOCATIONS
	Coff_Relocation reloc;
	reloc.addr = 0x04;
	reloc.sym_index = 0x01;
	reloc.type = RELOC_REL32;

	out_seek(0x70);
	out_write_t(reloc);

	// SYMBOLS
	Coff_Symbol sym;
	memzero(&sym, sizeof(sym));

	strcpy(sym.shrtname, "main");
	sym.ptr = 0x00;
	sym.section = 0x01;
	sym.type = SYMTYPE_FUNC;
	sym.storage_cls = SYMCLS_EXTERNAL;

	out_seek(0x80);
	out_write_t(sym);

	sym.longname.zeroes = 0;
	sym.longname.offset = 0x4;
	sym.ptr = 0;
	sym.section = 0;
	sym.type = SYMTYPE_FUNC;
	sym.storage_cls = SYMCLS_EXTERNAL;
	out_write_t(sym);

	// STRING TABLE
	out_write_u32(0x10);
	out_write("WriteFile", 12);

	out_flush(path);
}