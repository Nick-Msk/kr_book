#include <stdio.h>
#include <string.h>

#include "guard.h"
#include "log.h"
#include "common.h"
#include "error.h"
#include "checker.h"
#include "files.p8.5.h"

typedef struct Keys {
    bool              version;    // bool example
    bool              unbuf;
    const char       *infilename;
    const char       *outfilename;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){ .version = false, .unbuf = false, .infilename = 0, .outfilename = 0, __VA_ARGS__}

#define                 parse_value(c, name, func)\
case c:\
    if (argv[0][1] == '\0') {\
        if (argv[1]){\
            ptr = argv[1];\
            argv++;\
        } else\
             return userraise(-1, ERR_WRONG_PARAMETER, "-"#c" option without value (must be string followed), ex '-lstring' or '-l string'");\
    } else\
        ptr = argv[0] + 1;\
    ke->name = func(ptr);\
    argv[0] += strlen(argv[0]) - 1;\
    params++;\
break;

// note - name must be a part of Keys structure, c must be char literal
#define                 parse_string(c, name) parse_value(c, name, )

// note - name must be a part of Keys structure, c must be char literal
#define                 parse_int(c, name) parse_value(c, name, atoi)

// note - name must be a part of Keys structure, c must be char literal
#define                 parse_long(c, name) parse_value(c, name, atol)

#define                 parse_bool(c, name)\
case c:\
     ke->name = true;\
     params++;\
break;

#define                 parse_bool_false(c, name)\
case c:\
     ke->name = false;\
     params++;\
break;

static int              parse_keys(const char *argv[], Keys *ke){

    invraise(ke, "Zero ke!!! Error!"); // raise here
    logenter("...");

    int     argc = 1, params = 0;
    char    c;
    const char *ptr;

    while (*++argv != 0 && **argv == '-'){
        argc++;
        while ( (c = *++argv[0]) )
            switch (tolower(c)){
                parse_bool('v', version);
                parse_bool('u', unbuf);
                parse_string('f', infilename);
                parse_string('o', outfilename);
                default:    // probaly it's possible to ignore unknows parameters
                    return userraise(-1, ERR_WRONG_PARAMETER, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d, %s", params, argc, ptr = 0);
}

const char *usage_str = "Usage: %s -u -v -f <input filename> -o <output filename>\n";

int                     main(int argc, const char *argv[]){
    logsimpleinit("Start");

    Keys    ke = Keysinit();
    argc = parse_keys(argv, &ke);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version || ke.outfilename == 0 || ke.infilename == 0){
        printf("%s mcp for test p8.5 pg 187-190\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    MFILE   *in = 0, *out = 0;
    if ( (in = mopen(ke.infilename, "r") ) == 0)
        return userraise(2, ERR_UNABLE_OPEN_FILE_READ, "Unable to open file %s for read\n", ke.infilename);

    if ( (out = mopen(ke.outfilename, "w") ) == 0)
        return userraise(3, ERR_UNABLE_OPEN_FILE_WRITE, "Unable to open file %s for write\n", ke.outfilename);

    if (ke.unbuf)
        if (!munbuf(out) )
            return userraise(3, ERR_UNABLE_SET_FILE_PARAM, "Failed to setup unbuffered mode\n");

    int     c;
    while ( ( c = mgetc(in) ) != EOF && RGUARDM){
        MODEXEC(500, (void) logmsg("next 500, [%c]", c) );
        mputc(c, out);
    }

    mclose(in);
    mclose(out);

    return logret(0, "end...");  // as replace of logclose()
}

