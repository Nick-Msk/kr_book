#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

// to common.h
const char     *strreverse(char *str, int len);

const char     *itob(int val, char *buf, int bufsz, int base);

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


int             main(int argc, const char *argv[]){

    init_checker(1, 3, "Usage: %s value base \n", *argv);

    char    buf[64];
    int     val  = strtol(argv[1], 0, 10);   // TODO: assume my strtoval
    int     base = strtol(argv[2], 0, 10);

    // check(base < 2 || base > 16, 2, "Base = %d must be between 2 and 16\n", base);
    if (!check_between(base, 2,  16, "Base = %d must be between 2 and 16\n", base))
        return 2;

    if (!check_negative(val, "value must be positive\n"))
        return 3;

    printf("%d (in %d) = %s\n", val, base,
                            itob(val, buf, sizeof(buf), base));

    return 0;
}

static inline
char            char_to_symbol(int digit){
    // assuming positive 
    if (digit <= 10)
        return digit + '0';
    if (digit <= 16)
        return digit + 'A'; // configurable?
    return '\0';
}

const char     *itob(int val, char *buf, int bufsz, int base){
    int         i = 0, sign = 0; //, maxflag = 0;
    unsigned    uval;
    if  (base == 10 && (sign = val) < 0)
        uval = -val;
    else 
        uval = (unsigned) val;  // without sign!
    do {
        buf[i++] = char_to_symbol(uval % base); // + maxflag); // not sure  for 2...
        //maxflag = 0;
    } while (i < bufsz - 2 && (uval /= base) > 0);
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

