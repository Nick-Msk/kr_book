#include <stdio.h>
#include <string.h>

#include "log.h"
#include "fs.h"
#include "common.h"
#include "error.h"
#include "string_hash.h"
#include "getword.h"

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

static int              proc_defines(void);

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

    int cnt = proc_defines();
    if (cnt < 0)
        fprintf(stderr, "Procedding failed\n");
    else
        printf("Processed %d\n", cnt);

    if (!fs_alloc_check(false))
        logmsg("Warning: incorrect allocation of fs's");

    return logret(0, "end...");  // as replace of logclose()
}

static int              proc_defines(void){

    stringhash      h = strhash_create(200, HASH_SIMPLE);    // 200 not sure

    // do some tests here
    int     cnt = 0;
    fs      str = FS(), val = FS();
    bool    def_flag = false, undef_flag = false, print_flag = false;
    char   *name, *value;

    printf(">");
    while (getsimpleword(&str) ){
        if (def_flag){
            def_flag = false;
            name = fsstr(str);
            if (getsimpleword(&val) )
                value = fsstr(val);
            else
                continue;
            if (!strhash_install(&h, name, value) )
                fprintf(stderr, "Unable to install [%s:%s]\n", name, value);
            else
                printf("Installed! (%d)\n", ++cnt);
        }
        else if (undef_flag){
            undef_flag = false;
            name = fsstr(str);
            if (strhash_undef(&h, name) )
                printf("Undefined [%s]\n", fsstr(str) );
            else
                printf("Not found [%s]\n", fsstr(str) );
        }  else {
            if (strncmp(str.v, "define", 3) == 0)
                def_flag = true;
            else if (strncmp(str.v, "undef", 3) == 0)
                undef_flag = true;
            else if (strncmp(str.v, "printall", 3) == 0)
                strhash_print(&h);
            else if (strncmp(str.v, "quit", 1) == 0)
                break;
            else
                printf("Unknown command [%s]", str.v);
        }
        printf("\n>");
    }
    strhashfree(h);
    fsfree(str);
    fsfree(val);
    return cnt;
}

