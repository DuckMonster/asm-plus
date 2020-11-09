#include <stdio.h>
#include <stdlib.h>
#include "compile.h"
#include "log.h"
#include "output.h"
#include "coff.h"

int main(int argc, const char** argv)
{
	Compile_Manifest compile;
	compile_file(argv[1], &compile);
	coff_write(argv[2], &compile);
	system("pause");
	return;
}