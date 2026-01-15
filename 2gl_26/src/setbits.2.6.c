#include <stdio.h>
#include <stdlib.h>

#include "log.h"
#include "common.h"
#include "checker.h"

static int              setbits(unsigned x, unsigned p, unsigned n, unsigned y);

// to common.c
char*                   reverse(char *s, int len){
    logenter("[%s]", s);
    int i = 0, j = len - 1;
    while (i < j)
        char_exch(s + i++, s + j--);
    return logret(s, "Reversed: [%s]", s);
}

// to common.h
int                     fprint_bits(FILE *f, const char *str, unsigned val){
    char            buf[100];
    unsigned        pos = 0, bit;
    while (pos < sizeof(buf) - 1 && val > 0){
        bit = val & 0x1;
        buf[pos++] = bit + '0';
        val >>= 1;
    }
    buf[pos] = '\0';
    reverse(buf, pos);
    return fprintf(f, "%s: %s\n", str, buf);
}

// to common.h
static inline int       print_bits(const char *str, unsigned  val){
    return fprint_bits(stdout, str, val);
}

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

    int res = x; // setbits(x, p, n, y);
    printf("Res = %d\n", res);
    print_bits("Bits=", res);

    logclose("...");
    return 0;
}

static int          setbits(unsigned x, unsigned p, unsigned n, unsigned y){
    int res = x;
    return res;
}
