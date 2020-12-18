#pragma once
#include "token.h"
typedef struct Node_T Node;

enum
{
	LOG_DEV,
	LOG_TRIVIAL,
	LOG_MEDIUM,
	LOG_IMPORTANT,
	LOG_NONE,
};
extern u32 log_lvl;
extern u32 error_count;
extern u32 warning_count;

void log_write(u32 level, const char* msg, ...);
void log_writel(u32 level, const char* msg, ...);
void log_codeline(u32 level, Token token);
void error(const char* msg, ...);
void error_at(Token token, const char* msg, ...);

void warning_at(Token token, const char* msg, ...);