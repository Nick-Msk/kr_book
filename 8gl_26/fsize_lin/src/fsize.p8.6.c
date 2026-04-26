#include <stdio.h>
#include <string.h>
#include "fsize.h"

const char *usage_str = "Usage: %s dir_or_filename\n";

int                     main(int argc, const char *argv[]){

	int			cnt = 0;
	if (argc == 1)
		cnt += fsize(".");
	else 
		while (--argc > 0)
	 		cnt += fsize(*++argv);

    printf("Total: %d\n", cnt);

    return 0;  // as replace of logclose()
}

