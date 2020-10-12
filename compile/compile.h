#pragma once
#include "parse.h"

void compile_file(const char* path);
void compile_instruction(Token* token);

enum
{
	INST_NULL,
	INST_MOV,
};

u32 parse_inst(Token* token);
