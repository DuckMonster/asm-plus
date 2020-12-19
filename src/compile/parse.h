#pragma once
#include "token.h"

bool token_read(Token* out_token);
bool token_read_immedate(Token* out_token);

/* NODES */
enum
{
	NODE_NULL,
	NODE_INST,
	NODE_KEYWORD,
	NODE_LABEL,
	NODE_CONST,
	NODE_DEREF,
	NODE_RAW,
};

#define NODE_IMPL()\
u32 type;\
Token token;\
\
Node* prev;\
Node* next

typedef struct Node_T Node;
typedef struct Node_T
{
	NODE_IMPL();
} Node;
ARRAY_DEF(Node_Array, Node*);

Node* node_make(u32 size, Token token);
#define node_make_t(type, token) ((type*)node_make(sizeof(type), token))
void node_push(Node* base, Node* node);

Node* parse_expression();
Node* parse_label(Token token);

/* INSTRUCTION */
typedef struct
{
	NODE_IMPL();

	Node_Array args;
} Node_Instruction;

Node_Instruction* parse_instruction(Token token);
#define inst_arg(inst, idx) (inst->args.data[idx])

/* CONST */
typedef struct
{
	NODE_IMPL();

	u64 value;
	u8 size;
} Node_Const;
Node_Const* parse_const(Token token);

/* DEREFERENCE */
typedef struct
{
	NODE_IMPL();

	Node* expr;
} Node_Dereference;

Node_Dereference* parse_dereference(Token token);

/* RAW */
typedef struct
{
	NODE_IMPL();

	Node_Array values;
} Node_Raw;

Node_Raw* parse_raw(Token token);

/* PARSE */
typedef struct
{
	const char* path;
	Node root;
} Parse; 

void parse_file(const char* file, Parse* p);