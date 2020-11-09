#include "compile.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "output.h"
#include "log.h"
#include "input.h"

void compile_file(const char* path, Compile_Manifest* out_manifest)
{
	memzero(out_manifest, sizeof(Compile_Manifest));

	Section section;
	memzero_t(section);

	section.name = ".code";
	section.data_size = 32;
	section.data = malloc(32);
	memzero(section.data, 32);

	// Write section temp code
	// xor rbx, 0xDEADBEEF
	section.data[0] = 0x48;
	section.data[1] = 0x81;
	section.data[2] = 0b11110011;
	section.data[3] = 0xEF;
	section.data[4] = 0xBE;
	section.data[5] = 0xAD;
	section.data[6] = 0xDE;

	// call ExitProcess
	section.data[7] = 0xE8;
	section.data[8] = 0x60;

	// Write temp entry symbol
	Symbol entry_symbol;
	memzero_t(entry_symbol);

	entry_symbol.name = "my_entry";
	entry_symbol.name_len = 8;
	entry_symbol.ptr = 0;
	entry_symbol.type = SYM_EXPORT;
	array_add(section.symbols, entry_symbol);

	// Write temp relocation
	Relocation relocation;
	relocation.sym_index = 1;
	relocation.ptr = 8;
	relocation.size = 4;
	array_add(section.relocations, relocation);

	// Write temp ExitProcess symbol
	Symbol exit_symbol;
	memzero_t(exit_symbol);

	exit_symbol.name = "ExitProcess";
	exit_symbol.name_len = 11;
	exit_symbol.ptr = 0;
	exit_symbol.type = SYM_IMPORT;
	array_add(section.symbols, exit_symbol);

	array_add(out_manifest->sections, section);
}