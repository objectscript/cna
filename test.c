#include <stdio.h>

#include "cna.h"

int main(int argc, char **argv) 
{
	int args[1] = { 33 };
	int retval;
	call_function_from_dynamic_library(argv[1], argv[2], 1, args, &retval);
	printf("%d\n", retval);
	return 0;	
}