#include "compile.h"
#include "log.h"
#include "output.h"

// Registers
Register reg_list[MAX_REG] =
{
	{ "A", 0b111, 1 },
	{ "B", 0b000, 1 },
};

/* MOV */
void compile_mov_reg_c(Register dst, Node* dst_node, Constant src, Node* src_node)
{
	if (src.size > 1)
		warning_at(src_node->ptr, src_node->len, "Move constant bigger than 1 byte; will be truncated");

	log_writel(LOG_TRIVIAL, "MOV %s 0x%X", dst.name, src.value);
	out_write_bits(0b01, 2);
	out_write_bits(dst.code, 3);
	out_write_bits(0b110, 3);

	out_write_u8(src.value);
}

void compile_mov_reg_reg(Register dst, Node* dst_node, Register src, Node* src_node)
{
	log_writel(LOG_TRIVIAL, "MOV %s %s", dst.name, src.name);
	out_write_bits(0b01, 2);
	out_write_bits(dst.code, 3);
	out_write_bits(src.code, 3);
}

void compile_mov(Node_Instruction* inst)
{
	if (inst->num_args != 2) 
	{
		error_at(inst->ptr, inst->len, "Invalid argument count; expected 2, found %d", inst->num_args);
		return;
	}

	Node* dst = inst->args;
	Node* src = dst->next;

	// r < ?
	if (dst->type == NODE_KEYWORD)
	{
		Register dst_r;
		resolve_register(dst, &dst_r);

		// r < r'
		if (src->type == NODE_KEYWORD)
		{
			Register src_r;
			resolve_register(src, &src_r);

			compile_mov_reg_reg(dst_r, dst, src_r, src);
			return;
		}

		// r < n
		if (src->type == NODE_CONST || src->type == NODE_CONST_HEX || src->type == NODE_CONST_BIN)
		{
			Constant src_c;
			resolve_constant(src, &src_c);

			compile_mov_reg_c(dst_r, dst, src_c, src);
			return;
		}
	}

	error_at(inst->ptr, inst->len, "Invalid mov arguments");
}

/* JMP */
void compile_jmp_c(Constant c, Node* c_node)
{
	log_writel(LOG_TRIVIAL, "JMP 0x%04X", c.value);
	if (c.size > 2)
		warning_at(c_node->ptr, c_node->len, "jmp destination constant size %d; will be truncated", c.size);

	out_write_u8(0xC3);
	out_write_u16(c.value);
}

void compile_jmp_addr(Address addr, Node* addr_node)
{

}

void compile_jmp(Node_Instruction* inst)
{
	if (inst->num_args != 1)
	{
		error_at(inst->ptr, inst->len, "Invalid argument count; expected jump address", inst->num_args);
		return;
	}

	Constant dest;
	resolve_constant(inst->args, &dest);

	compile_jmp_c(dest, inst->args);
}

Instruction inst_list[MAX_INST] =
{
	{ "mov", compile_mov },
	{ "jmp", compile_jmp },
	{ NULL, NULL }
};