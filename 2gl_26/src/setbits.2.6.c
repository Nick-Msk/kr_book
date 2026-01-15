#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "common.h"
#include "checker.h"

static int              setbits(unsigned x, unsigned p, unsigned n, unsigned y);

int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/setbits.2.6.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir") 

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s setbits KR task 2.6 (input:  stdin)\nUsage: %s x, p, n, y (all unsigned)\n", __FILE__, *argv); // Usage here??
            return 0;
        }
    }
    if (!check_arg(4, "Usage: %s x, p, n, y (all unsigned)\n", *argv) ){
        return 1;
        }
    int x, p, n, y;
    x = atoi(argv[1]);
    p = atoi(argv[2]);
    n = atoi(argv[3]);
    y = atoi(argv[4]);

    if (!inv (x >= 0 && p >= 0 && n >= 0 && y >= 0, "must be positive") )
        return 2;

    int res = setbits(x, p, n, y);
    printf("Res = %d\n", res);
    print_bits("Bits=", res);

    logclose("...");
    return 0;
}

static int          setbits(unsigned x, unsigned p, unsigned n, unsigned y){
    int res = x;
    
    // TODO: 
    return res;
}

