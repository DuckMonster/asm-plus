#include "compile.h"
#include "log.h"
#include "output.h"

Register arg_reg(Node_Instruction* inst, u32 idx)
{
	Register reg;
	resolve_register(inst->args.data[idx], &reg);
	return reg;
}

void write_reg_byte(Register arg, Register target)
{
	if (target.dereference)
		out_write_bits(0b00, 2);
	else
		out_write_bits(0b11, 2);

	out_write_bits(arg.code, 3);
	out_write_bits(target.code, 3);
}

bool register_dereference_check(Register a, Register b)
{
	if (a.dereference && b.dereference)
	{
		error_at(token_join(a.token, b.token), "Invalid register dereference combination");
		return false;
	}

	return true;
}

void compile_jmp_cnst(Node_Instruction* inst)
{
	Constant cnst;
	resolve_constant(inst->args.data[0], &cnst);

	out_write_u8(0xE9);
	out_write_u32(cnst.value);
}

void compile_mov_reg_reg(Node_Instruction* inst)
{
	Register
		dst = arg_reg(inst, 0),
		src = arg_reg(inst, 1);

	if (!register_dereference_check(src, dst))
		return;

	if (src.dereference)
	{
		out_write_u8(0x8B);
		write_reg_byte(dst, src);
	}
	else
	{
		out_write_u8(0x89);
		write_reg_byte(src, dst);
	}
}

void compile_mov_reg_cnst(Node_Instruction* inst)
{
	Register dst;
	Constant src;
	resolve_register(inst->args.data[0], &dst);
	resolve_constant(inst->args.data[1], &src);

	out_write_u8(0xB8 + dst.code);
	out_write_u32(src.value);
}

// Instructions
Instruction instruction_list[MAX_INSTRUCTIONS] =
{
	{ "mov", 2, { ARG_REGISTER, ARG_REGISTER }, compile_mov_reg_reg },
	{ "mov", 2, { ARG_REGISTER, ARG_CONST }, compile_mov_reg_cnst },
	{ "jmp", 1, { ARG_CONST }, compile_jmp_cnst },
	{ NULL, 0, { 0 }, NULL },
};

// Registers
Register register_list[MAX_REGISTERS] =
{
	{ "a", 0b000 },
	{ "b", 0b011 },
	{ NULL, 0 },
};