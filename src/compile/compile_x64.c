#include "compile.h"
#include "log.h"
#include "output.h"

void compile_jmp_cnst(Node_Instruction* inst)
{
	Constant cnst;
	resolve_constant(inst->args.data[0], &cnst);

	out_write_u8(0xE9);
	out_write_u32(cnst.value);
}

void compile_mov_reg_reg(Node_Instruction* inst)
{
	Register src, dst;
	resolve_register(inst->args.data[0], &dst);
	resolve_register(inst->args.data[1], &src);

	out_write_u8(0x89);
	out_write_bits(0b11, 2);
	out_write_bits(src.code, 3);
	out_write_bits(dst.code, 3);
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
	{ "a", 0b000 },
	{ "b", 0b011 },
	{ NULL, 0 },
};