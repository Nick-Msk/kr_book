#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "common.h"
#include "checker.h"

static int              invert(unsigned x, unsigned p, unsigned n);

int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/invert.2.7.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir") 

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s invert KR task 2.7 (input:  stdin)\nUsage: %s x, p, n (all unsigned)\n", __FILE__, *argv); // Usage here??
            return 0;
        }
    }
    if (!check_arg(4, "Usage: %s x, p, n (all unsigned)\n", *argv) ){
        return 1;
        }
    int x, p, n;
    x = atoi(argv[1]);
    p = atoi(argv[2]);
    n = atoi(argv[3]);

    if (!inv (x >= 0 && p >= 0 && n >= 0, "must be positive") )
        return 2;

    int res = invert(x, p, n);
    printf("Res = %d\n", res);
    print_bits("Orig=", x);
    print_bits("Bits=", res);

    logclose("...");
    return 0;
}

static int          invert(unsigned x, unsigned p, unsigned n){

    char        buf[100];
    logenter("x = %s", bits_str(buf, sizeof(buf), x) );
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
    x |= y;

    return logret(x, "res = %s", bits_str(buf, sizeof(buf), x));
}

