#pragma once
enum
{
	LOG_TRIVIAL,
	LOG_MEDIUM,
	LOG_IMPORTANT,
	LOG_NONE,
};
extern u32 log_lvl;
extern u32 error_count;
extern u32 warning_count;

void log_write(u32 level, const char* msg, ...);
void error(const char* msg, ...);
void error_at(const char* ptr, u32 len, const char* msg, ...);

void warning_at(const char* ptr, u32 len, const char* msg, ...);