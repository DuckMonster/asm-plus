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

void compile_mov_reg_reg(Node_Instruction* inst)
{
	Register dst, src;
	resolve_register(inst->args[0], &dst);
	resolve_register(inst->args[1], &src);

	out_write_bits(0b01, 2);
	out_write_bits(dst.code, 3);
	out_write_bits(src.code, 3);
}

void compile_mov_reg_cmem(Register dst, Node* dst_node, Constant src, Node* src_node)
{
	if (src.size > 2)
		warning_at(src_node->ptr, src_node->len, "Move memory constant bigger than 2 bytes; will be truncated");

	out_write_bits(0b00, 2);
	out_write_bits(dst.code, 3);
	out_write_bits(0b010, 3);

	out_write_u16(src.value);
}

void compile_mov_reg_smem(Register dst, Node* dst_node, Symbol_Reference sym, Node* sym_node)
{
	out_write_bits(0b00, 2);
	out_write_bits(dst.code, 3);
	out_write_bits(0b010, 3);

	write_symbol_ref(sym);
}

/*
void compile_mov_cmem_reg(Constant dst, Node* dst_node, Register src, Node* src_node)
{
	if (dst.size > 2)
		warning_at(dst_node->ptr, dst_node->len, "Move memory constant bigger than 2 bytes; will be truncated");

	out_write_bits(0b00, 2);
	out_write_bits(dst.code, 3);
	out_write_bits(0b010, 3);

	out_write_u16(src.value);
}

void compile_mov_smem_reg(Symbol_Reference dst, Node* dst_node, Register sym, Node* sym_node)
{
	out_write_bits(0b00, 2);
	out_write_bits(dst.code, 3);
	out_write_bits(0b010, 3);

	write_symbol_ref(sym);
}

void compile_mov_cmem_c(Constant dst, Node* dst_node, Constant src, Node* src_node)
{
	if (dst.size > 2)
		warning_at(dst_node->ptr, dst_node->len, "Move memory constant bigger than 2 bytes; will be truncated");

	log_writel(LOG_TRIVIAL, "MOV %s [0x%04X]", dst.name, src.value);
	out_write_bits(0b00, 2);
	out_write_bits(dst.code, 3);
	out_write_bits(0b010, 3);

	out_write_u16(src.value);
}

void compile_mov_smem_c(Symbol_Reference dst, Node* dst_node, Constant sym, Node* sym_node)
{
	out_write_bits(0b00, 2);
	out_write_bits(dst.code, 3);
	out_write_bits(0b010, 3);

	write_symbol_ref(sym);
}
*/

void compile_mov(Node_Instruction* inst)
{
	/*
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
		if (src->type == NODE_CONST)
		{
			Constant src_c;
			resolve_constant(src, &src_c);

			compile_mov_reg_c(dst_r, dst, src_c, src);
			return;
		}

		// r < [?]
		if (src->type == NODE_MEM)
		{
			// Memory move only available for 'a'
			if (dst_r.code != 0b111)
			{
				error_at(src->ptr, src->len, "Memory mov only available for register 'a'");
				return;
			}

			Node_Memory* src_mem = (Node_Memory*)src;
			src = src_mem->expr;
			switch(src->type)
			{
				// r < [nn]
				case NODE_CONST:
					Constant src_c;
					resolve_constant(src, &src_c);

					compile_mov_reg_cmem(dst_r, dst, src_c, src);
					return;

				// r < [symbol]
				case NODE_KEYWORD:
					Symbol_Reference src_s;
					if (!resolve_symbol_ref(src, &src_s))
					{
						error_at(src->ptr, src->len, "Unresolved symbol '%.*s'", src->len, src->ptr);
						return;
					}

					compile_mov_reg_smem(dst_r, dst, src_s, src);
					return;
			}
		}
	}

	// [?] < ?
	if (dst->type == NODE_MEM)
	{

	}

	error_at(inst->ptr, inst->len, "Invalid mov arguments");
	*/
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

void compile_jmp_symbol(Symbol_Reference sym, Node* sym_node)
{
	log_writel(LOG_TRIVIAL, "JMP '%.*s'", sym.symbol->node->len, sym.symbol->node->ptr);
	out_write_u8(0xC3);
	write_symbol_ref(sym);
}

void compile_jmp(Node_Instruction* inst)
{
	if (inst->num_args != 1)
	{
		error_at(inst->ptr, inst->len, "Invalid argument count; expected jump address", inst->num_args);
		return;
	}

	Node* arg = inst->args;
	switch(arg->type)
	{
		case NODE_CONST:
			Constant dest;
			resolve_constant(arg, &dest);
			compile_jmp_c(dest, arg);
			break;

		case NODE_KEYWORD:
			Symbol_Reference sym;
			if (!resolve_symbol_ref(arg, &sym))
			{
				error_at(arg->ptr, arg->len, "Unresolved symbol '%.*s'", arg->len, arg->ptr);
				return;
			}

			compile_jmp_symbol(sym, arg);
			break;

		default:
			error_at(arg->ptr, arg->len, "Invalid jmp argument");
			break;
	}
}

Instruction inst_list[MAX_INST] =
{
	{ "mov", { ARG_REG, ARG_REG, 0 }, compile_mov_reg_reg },
	//{ "jmp", { ARG_CONST, 0 }, compile_jmp_c },
	{ NULL, { 0 }, NULL }
};