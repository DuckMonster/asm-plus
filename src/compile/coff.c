#include "coff.h"
#include "output.h"

void coff_write(const char* path)
{
	out_begin();

	Coff_Header hdr;
	memzero(&hdr, sizeof(hdr));

	hdr.machine = MACHINE_AMD64;
	hdr.section_num = 1;
	out_write_t(hdr);

	Coff_Section_Header s_hdr;
	memzero(&s_hdr, sizeof(s_hdr));

	strcpy(s_hdr.name, "code");
	s_hdr.size = 0x10;
	s_hdr.data_ptr = 0x60;
	out_write_t(s_hdr);

	out_seek(0x60);
	for(u32 i=0; i<0x10; ++i)
		out_write_u8(0);

	out_flush(path);
}