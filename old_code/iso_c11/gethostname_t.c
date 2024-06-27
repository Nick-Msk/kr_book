#include <stdio.h>
#include <unistd.h>
#include <common.h>
#include <limits.h>

int 	main(void){
	typeprint(_SC_HOST_NAME_MAX);
	typeprint(_POSIX_HOST_NAME_MAX);
}

