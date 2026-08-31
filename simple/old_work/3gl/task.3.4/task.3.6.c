#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

// to common.h
const char     *strreverse(char *str, int len);

// to common.h
// assuming argc as checked value!!!
#define         init_checker(retval, argcnt, msg, ...)\
    if (argc < (argcnt)){\
        fprintf(stderr, (msg), ##__VA_ARGS__);\
        return ((retval));\
    }

// to checker.h
// todo not pretty correct, (val) must be called one time
#define         check_between(val, lv, hv, msg, ...)\
    ( (val) < (lv) || (val) > (hv) ? fprintf(stderr, (msg), ##__VA_ARGS__), 0 : 1 )

#define         check_negative(val, msg, ...)\
    ( (val) < 0 ? fprintf(stderr, (msg), ##__VA_ARGS__), 0 : 1)

// check_lower()
// check_higher()
// check_typ()
// check_nan()

const char     *itoalpad(int val, int lpad, char *buf, int bufsz);

int             main(int argc, const char *argv[]){

    init_checker(1, 3, "Usage: %s val1 val2\n", *argv);

    char    buf[64];
    int     val1  = strtol(argv[1], 0, 10);   // TODO: assume my strtoval
    int     val2  = strtol(argv[2], 0, 10);

    printf("%d + %d = [%s]\n", val1, val2,
                            itoalpad(val1 + val2, 4, buf, sizeof(buf)));

    return 0;
}

const char     *itoalpad(int val, int lpad, char *buf, int bufsz){
    int     i = 0, sign;
    if  ((sign = val) < 0)
        val = -val;
    do {
        buf[i++] = val % 10 + '0';
    } while (i < bufsz - 2 && (val /= 10) > 0);
    if (sign < 0)
        buf[i++] = '-';
    while (i < bufsz - 1 && i < lpad)
        buf[i++] = ' ';
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

