#include <stdio.h>
#include <stdlib.h>

#include "common.h"

int     main(int argc, const char *argv[]){
    if (argc < 2){
        fprintf(stderr, "Usage: %s val\n", *argv);
        return 1;
    }
    int val = atoi(argv[1]);
    printf("%d\n", round_up_2(val));
    return 0;
}

