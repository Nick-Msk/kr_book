#include <stdio.h>
#include <string.h>

#include "log.h"
#include "common.h"
#include "error.h"
#include "checker.h"
#include "parse_keys.h"
#include "fsize.h"
#include "fs.h"

typedef struct Keys {
    bool              version;    // bool example
    const char       *dirname;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){ .version = false,.dirname = 0, __VA_ARGS__}
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
                parse_string('f', dirname);
                default:    // probaly it's possible to ignore unknows parameters
                    return userraise(-1, ERR_WRONG_PARAMETER, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d, %s", params, argc, ptr = 0);
}

const char *usage_str = "Usage: %s <val1:TYPE1> <val2:TYPE2>\n";

int                     main(int argc, const char *argv[]){
    logsimpleinit("Start");

    Keys    ke = Keysinit();
    argc = parse_keys(argv, &ke);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s KR: fsize p8.6 p191\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    fs  dir = fscopy(".");
    if (ke.dirname)
        fscpystr(dir, ke.dirname);

    printf("Total: %d\n", fsize(&dir) );

    fsfree(dir);
    if (!fs_alloc_check(false))
        logmsg("Warning: incorrect allocation of fs's");

    return logret(0, "end...");  // as replace of logclose()
}

