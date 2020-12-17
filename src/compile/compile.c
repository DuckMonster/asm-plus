#include "compile.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "output.h"
#include "log.h"
#include "input.h"

/* COMPILE */
void compile_parsed(Parse* parse, Compile* c)
{
	timer_push();
	log_writel(LOG_MEDIUM, " Compiling");

	memzero(c, sizeof(Compile));

	compile_begin_section(c, token_from_str("code"));

	Node* node = &parse->root;
	while(node)
	{
		switch(node->type)
		{
			case NODE_LABEL:
				compile_add_export(c, node->token);
				compile_mark_symbol(c, node->token);
			break;

			case NODE_RAW:
				compile_raw(c, (Node_Raw*)node);
			break;

			case NODE_INST:
				compile_instruction(c, (Node_Instruction*)node);
			break;
		}

		node = node->next;
	}

	compile_end_section(c);
	log_writel(LOG_MEDIUM, " Compile complete (%.2f ms)\n", timer_pop_ms());

/*
	Node* node = &parse->root;
	while(node)
	{
		printf("%.*s\n", node->token.len, node->token.ptr);
		node = node->next;
	}

	return;
	compile_begin_section(c, token_from_str("readonly"));

	compile_add_export(c, token_from_str("msg"));
	compile_mark_symbol(c, token_from_str("msg"));
	out_write("Hello!", 6);

	compile_end_section(c);

	compile_begin_section(c, token_from_str("code"));
	compile_add_export(c, token_from_str("main"));
	compile_add_import(c, token_from_str("ExitProcess"));
	compile_add_import(c, token_from_str("GetStdHandle"));
	compile_add_import(c, token_from_str("WriteFile"));

	compile_mark_symbol(c, token_from_str("main"));

	// sub rsp, 0x40
	out_write_u8(0x48);
	out_write_u8(0x83);
	out_write_u8(0b11101100);
	out_write_u8(0x40);

	// mov ecx, -11
	out_write_u8(0xB8 + 0b001);
	out_write_u32(-11);

	// call GetStdHandle
	out_write_u8(0xE8);
	compile_write_func_relocation(c, token_from_str("GetStdHandle"), 0);

	// mov ecx, eax
	out_write_u8(0x89);
	out_write_u8(0b11000001);

	// mov edx, msg
	out_write_u8(0x48);
	out_write_u8(0xB8 + 0b010);
	compile_write_mem_relocation(c, token_from_str("msg"), 0);

	// mov r8, 5
	out_write_u8(0x41);
	out_write_u8(0xB8 + 0b000);
	out_write_u32(5);

	// mov r9, rsp
	out_write_u8(0x49);
	out_write_u8(0x89);
	out_write_u8(0b11100001);

	//sub rsp, 0x40
	out_write_u8(0x48);
	out_write_u8(0x83);
	out_write_u8(0b11101100);
	out_write_u8(0x40);

	// call WriteFile
	out_write_u8(0xE8);
	compile_write_func_relocation(c, token_from_str("WriteFile"), 0);

	out_write_u8(0xE8);
	compile_write_func_relocation(c, token_from_str("ExitProcess"), 0);

	out_write_u8(0xC3);

	compile_end_section(c);
*/
}

/* SECTION */
Section* compile_begin_section(Compile* c, Token token)
{
	if (c->cur_section)
		error("Tried to start section before previous was ended");

	out_begin();
	Section* section = array_add_zero(c->sections);
	section->token = token;
	section->index = c->sections.count - 1;

	c->cur_section = section;
	return section;
}

void compile_end_section(Compile* c)
{
	c->cur_section->data_size = out_offset();
	out_flush_mem(&c->cur_section->data);

	c->cur_section = NULL;
}

/* RELOCATION */
void compile_write_mem_relocation(Compile* c, Token sym_token, u64 base)
{
	Symbol* sym = compile_find_symbol(c, sym_token);
	if (!sym)
	{
		error_at(sym_token, "Tried to write reloaction to unknown symbol");
		return;
	}

	Relocation rel;
	rel.sym_index = sym->index;
	rel.ptr = out_offset();
	rel.type = RELOC_MEM;
	array_add(c->cur_section->relocations, rel);

	out_write_u64(base);
}

void compile_write_func_relocation(Compile* c, Token sym_token, u32 base)
{
	Symbol* sym = compile_find_symbol(c, sym_token);
	if (!sym)
	{
		error_at(sym_token, "Tried to write reloaction to unknown symbol");
		return;
	}

	Relocation rel;
	rel.sym_index = sym->index;
	rel.ptr = out_offset();
	rel.type = RELOC_FUNC;
	array_add(c->cur_section->relocations, rel);

	out_write_u32(base);
}

/* SYMBOL */
Symbol* compile_find_symbol(Compile* c, Token token)
{
	for(u32 i = 0; i < c->symbols.count; ++i)
	{
		Symbol* sym = &c->symbols.data[i];
		if (token_eq(sym->token, token))
			return sym;
	}

	return NULL;
}

Symbol* compile_add_symbol(Compile* c, Token token)
{
	Symbol* sym = array_add_zero(c->symbols);
	sym->index = c->symbols.count - 1;
	sym->token = token;

	return sym;
}

void compile_add_import(Compile* c, Token token)
{
	Symbol* sym = compile_find_symbol(c, token);
	if (sym)
	{
		if (sym->type == SYM_IMPORT)
			return;
		else
		{
			error_at(token, "Import error, symbol already defined with different type");
			return;
		}
	}

	sym = compile_add_symbol(c, token);
	sym->type = SYM_IMPORT;
}

void compile_add_export(Compile* c, Token token)
{
	Symbol* sym = compile_find_symbol(c, token);
	if (sym)
	{
		error_at(token, "Symbol already defined");
		return;
	}

	sym = compile_add_symbol(c, token);
	sym->type = SYM_EXPORT;
	sym->section = c->cur_section->index;
}

void compile_mark_symbol(Compile* c, Token token)
{
	Symbol* sym = compile_find_symbol(c, token);
	if (!sym)
	{
		error_at(token, "Symbol not defined, how did this happen???");
		return;
	}

	if (sym->type != SYM_EXPORT)
	{
		error_at(token, "Trying to mark symbol, but it isn't an export");
		return;
	}

	sym->ptr = out_offset();
}

/* RAW */
void compile_raw(Compile* c, Node_Raw* raw)
{
	for(u32 i=0; i<raw->values.count; ++i)
	{
		Node* val = raw->values.data[i];
		switch(val->type)
		{
			case NODE_CONST:
				Node_Const* cnst = (Node_Const*)val;
				out_write(&cnst->value, cnst->size);
			break;
		}
	}
}

/* INSTRUCTION */
void compile_instruction(Compile* c, Node_Instruction* inst_node)
{
	log_writel(LOG_DEV, "Compiling instruction '%s'", token_to_str(inst_node->token));

	Instruction* inst;
	if (!resolve_instruction(inst_node, &inst))
	{
		error_at(inst_node->token, "Unknown instruction '%s'", token_to_str(inst_node->token));
		return;
	}

	inst->func(inst_node);
}

bool resolve_instruction(Node_Instruction* inst_node, Instruction** out_inst)
{
	// Compare name, and arguments
	for(u32 inst_idx = 0; inst_idx < MAX_INSTRUCTIONS; ++inst_idx)
	{
		Instruction* inst = &instruction_list[inst_idx];

		// End of instruction list
		if (inst->name == NULL)
			break;

		// Num arguments
		if (inst_node->num_args != inst->num_args)
			continue;

		// Compare argument types
		Node* arg = inst_node->args;
		for (u32 arg_idx = 0; arg_idx < inst->num_args; ++arg_idx)
		{
			if (get_arg_type(arg) != inst->args[arg_idx])
				goto arg_mismatch;

			arg = arg->next;
		}

		// .. oh, you're still here? Must mean youre the one!
		*out_inst = inst;
		return true;

arg_mismatch:
		continue;
	}

	return false;
}

u8 get_arg_type(Node* arg_node)
{
	switch(arg_node->type)
	{
		case NODE_CONST: return ARG_CONST;
		case NODE_KEYWORD: return ARG_REGISTER;
	}

	return ARG_NULL;
}

/* CONSTANT */
bool resolve_constant(Node* node, Constant* cnst)
{
	if (node->type != NODE_CONST)
		return false;

	if (cnst)
	{
		Node_Const* node_cnst = (Node_Const*)node;
		cnst->value = node_cnst->value;
		cnst->size = node_cnst->size;
	}

	return true;
}

/* REGISTER */
bool resolve_register(Node* node, Register** reg)
{
	if (node->type != NODE_KEYWORD)
		return false;

	for(u32 i=0; i<MAX_REGISTERS; ++i)
	{

	}

	if (reg)
	{
	}

	return false;
}