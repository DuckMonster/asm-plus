#include "parse.h"
#include "input.h"
#include <stdlib.h>

Node* parse_base = NULL;
Node* parse_cur = NULL;

void adv_line()
{
	// Advance until _at least_ the newline
	while(!in_eof() && !NEWLINE(in_ptr()))
		in_adv(1);

	// Then continue advancing until we find something interesting
	while(!in_eof() && (WHITESPACE(in_ptr()) || NEWLINE(in_ptr())))
		in_adv(1);
}

void adv_whitespace()
{
	while(!in_eof() && WHITESPACE(in_ptr()))
		in_adv(1);
}

bool token_read(Token* out_token)
{
	adv_whitespace();
	if (in_eof())
		return false;

	if (NEWLINE(in_ptr()))
		return false;

	// Keywords
	if (ALPHA(in_ptr()))
	{
		out_token->type = TOKEN_KEYWORD;
		out_token->ptr = in_ptr();
		out_token->len = 0;

		while(!in_eof() && ALPHA(in_ptr()))
		{
			in_adv(1);
			out_token->len++;
		}

		return true;
	}

	return false;
}

Node* parse_file(const char* file)
{
	in_load(file);
	parse_base = parse_cur = node_add_t(Node, NULL);

	// Read instruction
	Token token;
	while(token_read(&token))
	{
		Node_Instruction* inst = node_push_t(Node_Instruction, parse_base);
		inst->ptr = token.ptr;
		inst->len = token.len;
		inst->type = NODE_INST;

		adv_line();

		// Read arguments
		/*
		while(token_read(&token))
		{
			if (inst->args)
				node_push()
		}
		*/
	}

	return parse_base;
}

Node* node_add(u32 size, Node* base)
{
	Node* new_node = (Node*)malloc(size);
	memzero(new_node, size);

	if (base != NULL)
	{
		new_node->prev = base;
		if (base->next)
			base->prev = new_node;

		new_node->next = base->next;
		base->next = new_node;
	}

	return new_node;
}

Node* node_push(u32 size, Node* base)
{
	Node* new_node = (Node*)malloc(size);
	memzero(new_node, size);

	if (base != NULL)
	{
		while(base->next)
			base = base->next;

		new_node->prev = base;
		base->next = new_node;
	}

	return new_node;
}