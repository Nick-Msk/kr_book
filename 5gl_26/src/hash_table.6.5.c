#include <stdio.h>
#include <string.h>

#include "log.h"
#include "fs.h"
#include "common.h"
#include "error.h"
#include "string_hash.h"

typedef struct Keys {
    bool        version;    // bool example
    bool        string;
    char       *filename;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){ .version = false, .string = 0, __VA_ARGS__}

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
                case 'v':
                    if (!ke->version){
                        ke->version = true;
                        params++;
                    }
                break;
                case 'f':
                    ke->string = (char *) argv[1];        // save pointer
                    argv++;
                    argv[0] += strlen(argv[0]) - 1; // shift
                    params++;
                break;
                default:    // probaly it's possible to ignore unknows parameters
                    fprintf(stderr, "Illegal option [%c]\n", c);
                    return logerr(-1, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d", params, argc);
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
        printf("%s KR, hash table, task 6.5\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    // TODO: code from here!
    stringhash      h = strhash_create(200);    // 200 not sure
    if (!h){
        fprintf(stderr, "unable to create hash 200\n");
        return 2;
    }

    // do some tests here
    

    if (!fs_alloc_check(false))
        logmsg("Warning: incorrect allocation of fs's");

    return logret(0, "end...");  // as replace of logclose()
}

