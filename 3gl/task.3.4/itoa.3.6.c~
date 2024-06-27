#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

// to common.h
const char     *strreverse(char *str, int len);

const char     *itoa(int val, char *buf, int bufsz);

// to common.h
void            init_checker(int argc, int retval, int argccnt, const char *msg, ...);

int             main(int argc, const char *argv[]){

    init_checker(argc, 1, 3, "Usage: %s \n", *argv);    // TODO: revork that to MACRO

    char   buf[64];
    int     val1 = strtol(argv[1], 0, 10);   // TODO: assume my strtoval
    int     val2 = strtol(argv[2], 0, 10);
    int     sum = val1 + val2;

    printf("%d + %d = %s\n", val1, val2, 
                            itoa(sum, buf, sizeof(buf)));

    return 0;
}

const char     *itoa(int val, char *buf, int bufsz){
    int     i = 0, sign;
    if  ((sign = val) < 0)
        val = -val;
    do {
        buf[i++] = val % 10 + '0';
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

void            init_checker(int argc, int retval, int argccnt, const char *msg, ...){

    if (argc < argccnt){
        va_list ap;
        va_start(ap, msg);
        vfprintf(stderr, msg, ap);
        va_end(ap);
        exit(retval);
    }
}
