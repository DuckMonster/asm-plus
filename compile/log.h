#pragma once
enum
{
	LOG_TRIVIAL,
	LOG_MEDIUM,
	LOG_IMPORTANT,
	LOG_NONE,
};
extern u32 log_lvl;

void log(u32 level, const char* msg, ...);
void error(const char* msg, ...);