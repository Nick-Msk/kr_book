#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "getword.h"
#include "log.h"
#include "fs.h"
#include "tree_p6.5.h"

typedef struct Keys {
    bool    version;
    bool    sens;
    // ...
} Keys;
#define                 Keysinit(...) (Keys){ .version = false, .tolower = false, __VA_ARGS__}

static int              parse_keys(const char *argv[], Keys *ke){
    logenter("...");
    int     argc = 1, params = 0;
    if (!ke)
        return userraiseint(-1, "Zero ke!!! Error!");   // raise here
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
                case 'i':
                    if (!ke->tolower){
                        ke->tolowr = true;
                        params++;
                    }
                case 'f':
                     TODO:
                break;
                default:    // probaly it's possible to ignore unknows parameters
                    fprintf(stderr, "Illegal option [%c]\n", c);
                    return logerr(-1, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d", params, argc);

}

static const char   *usage_str = "Usage: %s -v -f -i\n";

int                      main(int argc, const char *argv[]){
    logsimpleinit("Start");

    Keys    ke = Keysinit();
    argc = parse_keys(argv, &ke);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s KR find any keywords p6.5\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    buffer_set(stdin);

    tnode       *root = 0;
    fs           word = FS();   // init empty with fsalloc

    while ( !fsisempty(word = getword(word, ke.sens) ) ) {
        if (isalpha_u(*fsstr(word) ) )
            root = tree_add(root, &word);
    }

    if (!fs_alloc_check(false))
        logmsg("Warning: incorrect allocation of fs's");

    return logret(0, "end...");  // as replace of logclose()
}


