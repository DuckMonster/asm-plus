#include "compile.h"
#include <stdio.h>
#include "log.h"
#include "input.h"

void compile_file(const char* path)
{
	log(LOG_MEDIUM, "Compiling '%s'", path);
	in_load(path);

	printf("%.*s\n", in_size(), in_ptr());
}