#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <limits.h>

// to common.h
const char     *strreverse(char *str, int len);

const char     *itoa(int val, char *buf, int bufsz);

// to common.h
//void            init_checker(int argc, int retval, int argccnt, const char *msg, ...);

// assuming argc as checked value!!!
#define         init_checker(retval, argcnt, msg, __VA_ARGS__)\
    if (argc < (argcnt)){\
        fprintf(stderr, (msg), __VA_ARGS__);\
        return ((retval));\
    }

int             main(int argc, const char *argv[]){

    //init_checker(argc, 1, 3, "Usage: %s \n", *argv);    // TODO: revork that to MACRO
    init_checker(1, 3, "Usage: %s \n", *argv);

    char    buf[64];
    int     val1 = strtol(argv[1], 0, 10);   // TODO: assume my strtoval
    int     val2 = strtol(argv[2], 0, 10);
    int     sum = val1 + val2;

    //printf("%d + %d = %s\n", val1, val2, 
      //                      itoa(sum, buf, sizeof(buf)));
    printf("MININT1 = %s\n", itoa(INT_MIN, buf, sizeof(buf)));
    return 0;
}

const char     *itoa(int val, char *buf, int bufsz){
    int     i = 0, sign, maxflag = 0;
    if (val == INT_MIN)
        maxflag = 1, val++;
    if  ((sign = val) < 0)
        val = -val;
    do {
        buf[i++] = val % 10 + '0' + maxflag;
        maxflag = 0;
    } while (i < bufsz - 2 && (val /= 10) > 0);
    if (sign < 0)
        buf[i++] = '-';
    buf[i] = '\0';
    return strreverse(buf, i - 1);
}

const char     *strreverse(char *str, int len){
    int     i, j;
    if (len == 0)
        len = strlen(str) - 1;
    char    c;
    for (i = 0, j = len; i < j; i++, j--){
        c = str[i];
        str[i] = str[j];
        str[j] = c;
    }
    return str;
}

