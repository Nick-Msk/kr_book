#include <unistd.h>
#include <stdio.h>

#include "log.h"
#include "common.h"
#include "bool.h"
#include "error.h"


static const int            in        = 0;
static const int            out       = 1;
static const int            BUFSIZE   = 1024;

const char                 *usage_str = "Usage: %s -b -v -g\n";

typedef struct Keys {
    bool        version;    // bool example
    bool        usegetchar;
    unsigned    bufsz;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){ .version = false, .bufsz = BUFSIZE, .usegetchar = false, __VA_ARGS__}

static int              parse_keys(const char *argv[], Keys *ke){
    logenter("...");
    int         argc = 1, params = 0;

    if (!ke)
        return userraiseint(ERR_WRONG_INPUT_PARAMETERS, "Zero ke!!! Error!");   // raise here
    char    c;
    while (*++argv != 0 && **argv == '-'){
        argc++;
        const char *ptr;
        while ( (c = *++argv[0]) )
            switch (tolower(c)){
                case 'v':
                    ke->version = true;
                    params++;
                break;
                case 'g':
                    ke->usegetchar = true;
                    params++;
                break;
                case 'b':
                    // TODO: refactor that to make more pretty
                    if (argv[0][1] == '\0') {
                        if (argv[1]){
                            ptr = argv[1];
                            argv++;
                        } else
                             return userraise(-1, ERR_WRONG_PARAMETER, "-l option without value (must be integer followed), ex '-l123' or '-l 123'");
                    } else  // argv[1] + 1 is pointer to value
                        ptr = argv[0] + 1;
                    ke->bufsz = atoi(ptr);        // save pointer
                    argv[0] += strlen(argv[0]) - 1; // shift
                    params++;
                break;
                default:    // probaly it's possible to ignore unknows parameters
                    return userraise(-1, ERR_WRONG_PARAMETER,  "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d", params, argc);
}

static bool         copy_bulk(unsigned bufsize);
static bool         copy_bychar(void);

int     main(int argc, const char *argv[]){
    logsimpleinit("Start");

    Keys    ke = Keysinit();
    argc = parse_keys(argv, &ke);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s file_operations, 8.2 p183\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    if (ke.usegetchar)
        copy_bychar();
    else
        copy_bulk(ke.bufsz);

    return logret(0, "end...");  // as replace of logclose()
}

static bool         copy_bulk(unsigned bufsize){
    char    buf[bufsize];
    int     cnt;

    while( (cnt = read(in, buf, bufsize) ) > 0)
        if (write(out, buf, cnt) < cnt){
            perror("Unable to write");
            return false;
        }
    return true;
}

static int          lgetchar(void){
    static char    buf[BUFSIZE];
    static char   *bufp = buf;
    static int     n = 0;

    if (n == 0){
        if ( (n = read(in, buf, BUFSIZE) ) < 0)
            userraiseint(ERR_STREAM_ERROR, "Error when read()");
        bufp = buf;
    }
    return (--n >= 0) ? (unsigned char) *bufp++ : EOF;
}

static int          lwritechar(int c){
    static char    buf[BUFSIZE];
    static char   *bufp = buf;
    static int     n = 0;

    if (n >= BUFSIZE || c == EOF){   // buffer is full or EOF (fflush)
        int tmp;
        if ( (tmp = write(out, buf, n) ) < n)
            userraiseint(ERR_STREAM_ERROR, "Error when write()");
        bufp = buf;
        n = 0;
    }
    // n < BUFSIZE here!
    if (c != EOF){
       *bufp++ = (unsigned char) c;
        n++;
    }
    return c;
}

static bool         copy_bychar(void){
    int     c;
    while ( (c = lgetchar()) != EOF)
        lwritechar(c);
    lwritechar(EOF);  // fflush
    return true;
}

