#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "log.h"
#include "common.h"
#include "checker.h"

static int              rightrot(unsigned x, unsigned n);

static const int        INT_WIDTH_BIT =  __INT_WIDTH__; // 32 actually

int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/rightrot.2.8.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir") 

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s rightrot KR task 2.8 (input:  stdin)\nUsage: %s x, n (all unsigned)\n", __FILE__, *argv); // Usage here??
            return 0;
        }
    }
    if (!check_arg(3, "Usage: %s x, n (all unsigned)\n", *argv) ){
        return 1;
        }
    int x, n;
    x = atoi(argv[1]);
    n = atoi(argv[2]);

    if (!inv (x >= 0 && n >= 0, "must be positive") )
        return 2;

    int res = rightrot(x, n);
    printf("Res = %d\n", res);
    print_bits("Orig=", x);
    print_bits("Bits=", res);

    logclose("...");
    return 0;
}

static int          rightrot(unsigned x, unsigned n){

    char        buf[100];
    logenter("x = %s", bits_str(buf, sizeof(buf), x) );

    n %= INT_WIDTH_BIT;
    unsigned cpy = x;
    x >>= n;

    unsigned mask = 0, i = n;
    while (i > 0)
        mask |= (1 << --i);
    cpy = (cpy & mask) << (INT_WIDTH_BIT - n);
    fprint_bits(logfile, "cpy with mask=", cpy);
    x |= cpy;
/*
    unsigned mask = 0;
    unsigned i = n;
    while (i > 0)
        mask |= (1 << (p + --i) );
    fprint_bits(logfile, "mask=", mask);

    unsigned y = x & mask;
    y = (~y) & mask; // insert
    fprint_bits(logfile, "inverted positions=", y);

    x &= ~mask; // free positions
    fprint_bits(logfile, "x before setup", x);
    x |= y;*/

    return logret(x, "res = %s", bits_str(buf, sizeof(buf), x));
}

