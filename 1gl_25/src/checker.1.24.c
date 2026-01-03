#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bool.h"
#include "log.h"

bool            check_input(int *errnum);

int             main(void){
   if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s simple error checker task 1.24 KR\nUsage: %s\n", __FILE__, *argv);
            return 0;
        }
    }

    int errnum = 0;
    if (check_input(&ernum))
        printf("Checks are passed\n");
    else
        printf("Error're occurs (%d)\n", errnum);

    return 0;
}

bool            check_input(int *p_errnum){
    int err = 0;

    // TODO: 

    if (p_errnum)
        *p_errnum = err;
    return err != 0;
}
