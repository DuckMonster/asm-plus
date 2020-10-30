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
	hdr.symbol_ptr = 0x70;
	hdr.symbol_num = 1;

	out_write_t(hdr);

	// CODE SECTION
	Coff_Section sect;
	memzero(&sect, sizeof(sect));

	strcpy(sect.name, "code");
	sect.size = 0x10;
	sect.data_ptr = 0x60;
	sect.flags = SCT_EXEC_CODE;

	out_write_t(sect);

	out_seek(0x60);
	out_write_u8(0x48);
	out_write_u8(0x33);

	out_write_bits(0b11, 2);
	out_write_bits(0b000, 3);
	out_write_bits(0b011, 3);

	out_write_u8(0xC3);

	// SYMBOLS
	Coff_Symbol sym;
	memzero(&sym, sizeof(sym));

	strcpy(sym.shrtname, "main");
	sym.ptr = 0x00;
	sym.section = 0x01;
	sym.type = SYMTYPE_FUNC;
	sym.storage_cls = SYMCLS_EXTERNAL;

	out_seek(0x70);
	out_write_t(sym);

	out_write_u32(0x4);

	out_flush(path);
}