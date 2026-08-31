#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "log.h"
#include "common.h"
#include "checker.h"
#include "bool.h"

typedef struct Keys {
    bool    except;
    bool    line;
    bool    version;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){.except = false, .line = false, .version = false, __VA_ARGS__}

static                  int parse_keys(const char *argv[], Keys *ke){
    logenter("...");
    int     argc = 1, params = 0;
    if (!ke)
        return logerr(-1, "Zero ke!!! Error!");
    char    c;
    while (*++argv != 0 && **argv == '-'){
        logauto(*argv);
        argc++;
        while ( (c = *++argv[0]) )
            switch (tolower(c)){
                case 'x':
                    if (!ke->except){
                        ke->except = true;
                        params++;
                    }
                break;
                case 'v':
                    if (!ke->version){
                        ke->version = true;
                        params++;
                    }
                break;
                case 'n':
                    if (!ke->line){
                        ke->line = true;
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

const char *usage_str = "Usage: %s <pattert:str>\n";

int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/"__FILE__".log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    Keys ke = Keysinit();
    argc = parse_keys(argv, &ke);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s KR grep p5.10 p128\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    static const int        MaxLine = 8000;
    char                    buf[MaxLine];
    const char             *pt = argv[argc];
    int                     found = 0;
    int                     lineno = 0;

    while (get_line(buf, MaxLine) > 0){
        lineno++;
        if ( (strstr(buf, pt) != 0) != ke.except){
            if (ke.line)
                printf("%d:", lineno);
            printf("%s", buf);
            found++;
        }
    }
    printf("\nTotal %d\n", found);
    logclose("found %d", found);
    return 0;
}

