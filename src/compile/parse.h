#pragma once

/* TOKEN PARSING */
enum
{
	TOKEN_KEYWORD = 256,
	TOKEN_CONST,
	TOKEN_CONST_HEX,
	TOKEN_CONST_BIN,
};

#define WHITESPACE(ptr) ((*(ptr)) == ' ' || (*(ptr)) == '\t')
#define NEWLINE(ptr) ((*(ptr)) == '\n' || (*(ptr)) == '\r')
#define ALPHA(ptr) (((*(ptr)) >= 'a' && (*(ptr)) <= 'z') || ((*(ptr)) >= 'A' && (*(ptr)) <= 'Z') || (*(ptr)) == '_' || (*(ptr)) == '-')
#define DIGIT(ptr) ((*(ptr)) >= '0' && (*(ptr)) <= '9')
#define HEX(ptr) (((*(ptr)) >= '0' && (*(ptr)) <= '9') || ((*(ptr)) >= 'A' && (*(ptr)) <= 'F') || ((*(ptr)) >= 'a' && (*(ptr)) <= 'f'))
#define BIN(ptr) ((*(ptr)) == '0' || (*(ptr)) == '1')
#define COMMENT(ptr) (*(ptr) == ';')

typedef struct
{
	u32 type;
	const char* ptr;
	u32 len;
} Token;

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
	NODE_MEM,
};

#define NODE_IMPL()\
u32 type;\
\
const char* ptr;\
u32 len;\
\
Node* prev;\
Node* next

typedef struct Node_T Node;
typedef struct Node_T
{
	NODE_IMPL();
} Node;

Node* node_make(u32 size, Token token);
#define node_make_t(type, token) ((type*)node_make(sizeof(type), token))
void node_push(Node* base, Node* node);

Node* parse_file(const char* file);

typedef struct Node_Instruction_T
{
	NODE_IMPL();

	u32 num_args;
	Node* args;
} Node_Instruction;

Node_Instruction* parse_instruction(Token token);

Node* parse_expression();
Node* parse_label(Token token);

typedef struct
{
	NODE_IMPL();

	u64 value;
	u8 size;
} Node_Const;
Node_Const* parse_const(Token token);

typedef struct
{
	NODE_IMPL();

	Node* expr;
} Node_Memory;

Node_Memory* parse_memory(Token token);