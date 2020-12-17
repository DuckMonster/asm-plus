#include "compile.h"
#include "log.h"

void compile_jmp_cnst(Node_Instruction* inst)
{
	log_writel(LOG_DEV, "jmp baby!");
}

void compile_mov_reg_reg(Node_Instruction* inst)
{
	log_writel(LOG_DEV, "mov baby!");
}

// Instructions
Instruction instruction_list[MAX_INSTRUCTIONS] =
{
	{ "mov", 2, { ARG_REGISTER, ARG_REGISTER }, compile_mov_reg_reg },
	{ "jmp", 1, { ARG_CONST }, compile_jmp_cnst },
	{ NULL, 0, { 0 }, NULL }
};

// Registers
Register register_list[MAX_REGISTERS] =
{
	{ "a", 0b001 },
	{ NULL, 0 },
};