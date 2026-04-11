#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "log.h"
#include "common.h"
#include "bool.h"
#include "error.h"

static const int            MAX_LINE = 80;
const char                 *usage_str = "Usage: %s -l(default 80) -h\n";

typedef struct Keys {
    bool        version;    // bool example
    int         maxline;
    bool        hex;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){ .version = false, .maxline = MAX_LINE, .hex = false, __VA_ARGS__}

static int              parse_keys(const char *argv[], Keys *ke){
    logenter("...");
    int         argc = 1, params = 0;
    const char *ptr;

    if (!ke)
        return userraiseint(ERR_WRONG_INPUT_PARAMETERS, "Zero ke!!! Error!");   // raise here
    char    c;
    while (*++argv != 0 && **argv == '-'){
        argc++;
        while ( (c = *++argv[0]) )
            switch (tolower(c)){
                case 'v':
                    if (!ke->version){
                        ke->version = true;
                        params++;
                    }
                break;
                case 'h':
                    if (!ke->hex){
                        ke->hex = true;
                        params++;
                    }
                break;
                case 'l':
                    // TODO: refactor that to make more pretty
                    if (argv[0][1] == '\0') {
                        if (argv[1]){
                            ptr = argv[1];
                            argv++;
                        } else
                             return userraise(-1, ERR_WRONG_PARAMETER, "-l option without value (must be integer followed), ex '-l123' or '-l 123'");
                    } else  // argv[1] + 1 is pointer to value
                        ptr = argv[0] + 1;
                    ke->maxline = atoi(ptr);        // save pointer
                    logauto(ptr);
                    argv[0] += strlen(argv[0]) - 1; // shift
                    params++;
                break;
                default:    // probaly it's possible to ignore unknows parameters
                    return userraise(-1, ERR_WRONG_PARAMETER,  "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d", params, argc);
}

// \0123
static inline void      print_octal(unsigned c){
    printf("\\0%c%c%c", itoc(c / 64), itoc( (c % 64) / 8), itoc(c % 8) );
}
// \x1F
static inline void      print_hex(unsigned c){
    printf("\\x%c%c", itohex(c / 16), itohex(c % 16) );
}

int                     main(int argc, const char *argv[]){
    logsimpleinit("Start");

    Keys    ke = Keysinit();
    argc = parse_keys(argv, &ke);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s formatter 7.2\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    int  c, len = 1;

    while ( (c = getchar() ) != EOF){
        if (c == '\n')
            putchar(c), len = 0;
        else if (c == '\t')
            printf("\\c");
        else if (isprint(c))
            putchar(c);
        else {
            if (ke.hex)
                print_hex(c);
            else
                print_octal(c);
        }
        if (++len % ke.maxline == 0){
            printf(" >>>>>\n");
            len = 1;
        }
    }

    return logret(0, "end...");  // as replace of logclose()
}

