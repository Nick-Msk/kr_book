#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

#include "log.h"
#include "common.h"
#include "error.h"

typedef struct Keys {
    bool        version;    // bool example
    bool        get_double;
    bool        get_float;
    bool        get_int;
    bool        get_unsigned;
    bool        get_long;
    bool        get_unsigned_long;
    bool        get_string;
    bool        get_pointer;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){\
    .version = false, .get_double = false, .get_float = false, .get_int = false, .get_unsigned = false, .get_long = false, .get_unsigned_long = false,\
    .get_string = false, .get_pointer = false, __VA_ARGS__}

static int              parse_keys(const char *argv[], Keys *ke){
    logenter("...");
    int     argc = 1, params = 0;
    if (!ke)
        return userraiseint(ERR_WRONG_INPUT_PARAMETERS, "Zero ke!!! Error!");   // raise here
    char    c;
    while (*++argv != 0 && **argv == '-'){
        argc++;
        while ( (c = *++argv[0]) )
            switch (tolower(c)){
                char        c1;
                case 'v':
                    if (!ke->version){
                        ke->version = true;
                        params++;
                    }
                break;
                case 'f':
                    ke->get_float = true;
                    params++;
                break;
                case 'd':
                    ke->get_int = true;
                    params++;
                break;
                case 'l':
                    c1 = *++argv[0];
                    switch (c1){
                        case 'd':
                            ke->get_long = true;
                        break;
                        case 'u':
                            ke->get_unsigned_long = true;
                        break;
                        case 'f':
                            ke->get_double = true;
                        break;
                        default:
                            return userraise(-1, ERR_WRONG_PARAMETER, "Illegal [%c %c], params [%d] argc %d", c, c1, params, argc);
                    }
                    params++;
                break;
                case 'u':
                    ke->get_unsigned = true;
                    params++;
                break;
                case 's':
                    ke->get_string = true;
                    params++;
                break;
                case 'p':
                    ke->get_pointer = true;
                    params++;
                break;
                default:    // probaly it's possible to ignore unknows parameters
                    return userraise(-1, ERR_WRONG_PARAMETER, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d", params, argc);
}

const char *usage_str = "Usage: -d -u -f -ld -lu -lf -s -p%s\n";

static int              miniscanf(const char *fmt, ...);  __attribute__ ((format (printf, 1, 2)));
static int              check_int(const Keys *ke);
static int              check_unsigned(const Keys *ke);

int                     main(int argc, const char *argv[]){
    logsimpleinit("Start");

    Keys    ke = Keysinit();
    argc = parse_keys(argv, &ke);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s miniscanf 7.4\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    double          d;
    float           f;
    int             i;
    long            l;
    char            str[256];    // omg

    printf("Type a int, then double, then long, then float: ");
    printf("Total get: %d\n", miniscanf("%d %lf %ld %f", &i, &d, &l, &f) );

    printf("String: ");
    miniscanf("%s", str);

    printf("Result: double %lf, int %d, long %ld,  float %f, string %s\n", d, i, l, f, str);

    check_int(&ke);
    check_unsigned(&ke);
    
    return logret(0, "end...");  // as replace of logclose()
}

#define                 check_type(type, fmt)\
static type              check_##type(void)\
    type     var;\
    if (ke->get_##type){\
        printf("Type a " #type ": ");\
        miniscanf(fmt, &var);\
        printf("Check res " fmt, var);\
    }\
}

static int              check_int(const Keys *ke){
    int     i;
    if (ke->get_int){
        printf("Type a int: ");
        miniscanf("%d", &i);
        printf("Check res [%d]", i);
        return 1;
    } else
        return 0;
}

static int              check_unsigned(const Keys *ke){
    int     i;
    if (ke->get_unsigned){
        printf("Type a unsigned: ");
        miniscanf("%u", &i);
        printf("Check res [%u]", i);
        return 1;
    } else
        return 0;
}
static int              miniscanf(const char *fmt, ...) {

    double          *d;
    float           *f;
    int             *i;
    short           *sh;
    long            *l;
    char            *s; // not array, but just the pointer
    void          **pt;
    char            *c;
    unsigned        *u;
    unsigned long   *lu;
    unsigned short  *su;
    //
    char            sym;    // for iter
    int             cnt = 0;
    va_list         ap;
    bool            breakflag = false;

    va_start(ap, fmt);

    for (const char *p = fmt; !breakflag && *p;  p++){
        sym = *p;
        if (isspace(sym) )
            continue;
        if (sym != '%'){
            char tmp;
            while ( (tmp = getchar() ) != EOF && isspace(tmp) )
                 ;
            if (tmp != sym)
                break;
        }
        switch ( *++p){
            case 'd':
                i = va_arg(ap, int *);
                scanf("%d", i);
                cnt++;
            break;
            case 'l':
                if (p[1] == 'd'){
                    l = va_arg(ap, long *);
                    scanf("%ld", l);
                    p++, cnt++;
                } else if (p[1] == 'u'){
                    lu = va_arg(ap, unsigned long *);
                    scanf("%lu", lu);
                    p++, cnt++;
                } else if (p[1] == 'f'){
                    d = va_arg(ap, double *);
                    scanf("%lf", d);
                    p++, cnt++;
                }
                 else {
                    logsimple("Incorrent format %c%c%c", '%', p[0], p[1]);
                    breakflag = true;
                }
            break;
            case 'h':
                if (p[1] == 'd'){
                    sh = va_arg(ap, short *);
                    scanf("%hd", sh);
                    p++, cnt++;
                } else if (p[1] == 'u'){
                    su = va_arg(ap, unsigned short *);
                    scanf("%hu", su);
                    p++, cnt++;
                } else {
                    logsimple("Incorrent format %c%c%c", '%', p[0], p[1]);
                    breakflag = true;
                }
            break;
            case 's':
                // 256 limit must be cheked
                logsimple("get a string!");
                s = va_arg(ap, char *);
                scanf("%s", s);
                //while ( (sym = getchar() ) != EOF && sym != '\n')
                  //  *s++ = sym;
                //*s = '\0';
                cnt++;
            break;
            case 'p':
                pt = va_arg(ap, void **);
                scanf("%p", pt);
                cnt++;
            break;
            case 'u':
                u = va_arg(ap, unsigned *);
                scanf("%u", u);
                cnt++;
            break;
            case 'c':
                c = va_arg(ap, char *);
                scanf("%c", c);
                cnt++;
            break;
            case 'f':
                f = va_arg(ap, float *);
                scanf("%f", f);
                cnt++;
            break;
            default:
                breakflag = true;
            break;
        }
    }
    va_end(ap);
    return cnt;
}

