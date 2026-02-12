#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "log.h"
#include "common.h"
#include "checker.h"
#include "bool.h"
#include "fs.h"
#include "fileutils.h"

typedef struct Keys {
    int     lines;
    bool    version;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){.version = false, __VA_ARGS__}

static int              parse_keys(const char *argv[], Keys *ke){
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
                case 'v':
                    if (!ke->version){
                        ke->version = true;
                        params++;
                    }
                break;
                case 'n':
                    if (!ke->lines){
                        ke->lines = atoi(argv[0]);
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

static int              tail(FILE *in, int sz);

const char *usage_str = "Usage: %s -n<cnt:int>\n";

int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/"__FILE__".log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    Keys ke = Keysinit(.lines = 10);
    //argc = parse_keys(argv, &ke);
    logauto(ke.lines);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s KR tail task 5.13\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    FILE        *f = stdin;
    const char  *fname = argv[argc];;

    if (fname && (f = fopen(fname, "r") ) == 0){
        fprintf(stderr, "Unable to open [%s] for read\n", fname);
        return 2;
    }

    int cnt = tail(f, ke.lines);

    if (f != stdin)
        fclose(f);

    logclose("%d", cnt);
    return 0;
}

// TODO: move that into common.h
static inline int       cycleinc(int val, int cycle){
    if (val >= cycle - 1)
        val = 0;
    else
        val++;
    return val;
}

static int              tail(FILE *in, int sz){
    logenter("sz = %d", sz);
    int     pend = 0;   // cycle
    bool    fill = false;
    fs      arr[sz];    // heap!
    int     cnt = 0;

    for (int i = 0; i < sz; i++)
        arr[i] = fsempty();

    while ( fgetline_fs(in, arr + pend) > 0){
        if (pend == sz - 1)
            fill = true;
        pend = cycleinc(pend, sz);
    }
    logmsg("pend = %d, fill = %s", pend, bool_str(fill));
    // now print the data
    if (pend != 0 || fill) { // no one line
        int i = fill ? pend : 0;
        do {
            printf("%d:%s", i, arr[i].v);
            i = cycleinc(i, sz);
        } while (i != pend );
    }
    return logret(cnt, "%d", cnt);
}
