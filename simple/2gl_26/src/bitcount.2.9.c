#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "common.h"
#include "checker.h"

static int              bitcount(unsigned x);


int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/bitcount.2.9.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir") 

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s bitcount KR task 2.9 (input:  stdin)\nUsage: %s x (unsigned)\n", __FILE__, *argv); // Usage here??
            return 0;
        }
    }
    if (!check_arg(2, "Usage: %s x (unsigned)\n", *argv) ){
        return 1;
        }
    int x;
    x = atoi(argv[1]);

    if (!inv (x >= 0, "must be positive") )
        return 2;

    int res = bitcount(x);
    printf("Res = %d\n", res);
    print_bits("Orig=", x);

    logclose("...");
    return 0;
}

static int          bitcount(unsigned x){
    logenter("x=%u", x);
    int res = 0;
    while (x > 0){
        fprint_bits(logfile, "  x=", x);
        fprint_bits(logfile, "x-1=", x - 1);
        x &= (x - 1);
        res++;
    }
    return logret(res, "%d", res);
}

