#pragma once
enum
{
	LOG_TRIVIAL,
	LOG_MEDIUM,
	LOG_IMPORTANT,
	LOG_NONE,
};
extern u32 log_lvl;

void log_write(u32 level, const char* msg, ...);
void error(const char* msg, ...);
void error_at(const char* ptr, u32 len, const char* msg, ...);