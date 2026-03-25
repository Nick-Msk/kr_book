#include <stdio.h>
#include <strings.h>
#include <ctype.h>

#include "log.h"
#include "common.h"
#include "fs.h"
#include "fs_iter.h"
#include "error.h"
#include "buffer.h"

typedef struct Keys {
    bool    version;
    bool    sens;
    // ...
} Keys;
#define                 Keysinit(...) (Keys){ .version = false, .sens = true, __VA_ARGS__}

static int              parse_keys(const char *argv[], Keys *ke){
    logenter("...");
    int     argc = 1, params = 0;
    if (!ke)
        return userraiseint(-1, "Zero ke!!! Error!");   // raise here
    char    c;
    while (*++argv != 0 && **argv == '-'){
        logauto(*argv);
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
                    if (!ke->sens){
                        ke->sens = false;
                        params++;
                    }
                break;
                default:    // probaly it's possible to ignore unknows parameters
                    fprintf(stderr, "Illegal option [%c]\n", c); 
                    return logerr(-1, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d", params, argc);

}

static const char   *usage_str = "Usage: %s -v -i\n";

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

    return logret(0, "end...");  // as replace of logclose()
}


