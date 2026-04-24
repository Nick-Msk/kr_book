#include <stdio.h>
#include <string.h>

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

static int              parse_keys(const char *argv[], Keys *ke){

    invraise(ke, "Zero ke!!! Error!"); // raise here

    logenter("...");

    int     argc = 1, params = 0;
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
                case 'f':
                    // TODO: refactor that to make more pretty
                    if (argv[0][1] == '\0') {
                        if (argv[1]){
                            ptr = argv[1];
                            argv++;
                        } else
                             return userraise(-1, ERR_WRONG_PARAMETER, "-l option without value (must be integer followed), ex '-l123' or '-l 123'");
                    } else  // argv[1] + 1 is pointer to value
                        ptr = argv[0] + 1;
                    ke->infilename = ptr;        // save pointer
                    argv[0] += strlen(argv[0]) - 1; // shift
                    params++;
                break;
                case 'o':
                    // TODO: refactor that to make more pretty
                    if (argv[0][1] == '\0') {
                        if (argv[1]){
                            ptr = argv[1];
                            argv++;
                        } else
                             return userraise(-1, ERR_WRONG_PARAMETER, "-l option without value (must be integer followed), ex '-l123' or '-l 123'");
                    } else  // argv[1] + 1 is pointer to value
                        ptr = argv[0] + 1;
                    ke->outfilename = ptr;        // save pointer
                    argv[0] += strlen(argv[0]) - 1; // shift
                    params++;
                break;
                default:    // probaly it's possible to ignore unknows parameters
                    return userraise(-1, ERR_WRONG_PARAMETER, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d", params, argc);
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
    if ( (in = mopen(ke.infilename, "r") ) != 0)
        return userraise(2, ERR_UNABLE_OPEN_FILE_READ, "Unable to open file %s for read\n", ke.infilename);
    if ( (out = mopen(ke.outfilename, "w") ) != 0)
        return userraise(3, ERR_UNABLE_OPEN_FILE_WRITE, "Unable to open file %s for write\n", ke.outfilename);

    int     c;
    //while ()
      //  mputc(c, );

    mclose(in);
    mclose(out);

    return logret(0, "end...");  // as replace of logclose()
}

