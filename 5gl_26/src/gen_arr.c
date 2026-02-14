#include <stdio.h>
#include <strings.h>
#include <ctype.h>

#include "log.h"
#include "checker.h"
#include "common.h"
#include "fileutils.h"
#include "fs.h"
#include "array.h"

typedef struct Keys {
    int     lines;
    bool    version;
    char    type;   // R - random A - ascending D - descending
    char   *fname;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){.version = false,.type = 'r', .lines = -1, .fname = 0, __VA_ARGS__}

static int              parse_keys(const char *argv[], Keys *ke){
    logenter("...");
    int     argc = 1, params = 0;
    char   *pos;

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
                    ke->lines = strtol(++argv[0], &pos, 10);
                    argv[0] = pos - 1; // -1 to break next while()
                    params++;
                break;
                case 't':
                    ke->type = *++argv[0];
                    params++;
                break;
                case 'f':
                    ke->fname = (char *) argv[0];        // save pointer
                    argv[0] += strlen(argv[0] - 1);
                    params++;
                break;
                default:    // probaly it's possible to ignore unknows parameters
                    fprintf(stderr, "Illegal option [%c]\n", c);
                    return logerr(-1, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d", params, argc);
}

static const char   *usage_str = "Usage: %s -t<type:char> -n<cnt:int>\n";

int                             main(int argc, const char *argv[]){
    static const char *logfilename = "log/"__FILE__".log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    Keys            ke = Keysinit();
    ArrayFillType   typ = ARRAY_RND;
    int             cnt = 10000;
    const char     *fname = "result/gen1.dat";

    argc = parse_keys(argv, &ke);
    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s KR gen_array\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    if (ke.lines > 0)
        cnt = ke.lines;
    if (ke.fname)
        fname = ke.fname;

    Array arr = DArray_create(cnt, typ);

    if (Array_savevalues(arr, fname, '\n') < 0){
        fprintf(stderr, "Unable to open [%s] for writing\n", fname);
        Arrayfree(arr);
        return 2;
    }

    Arrayfree(arr);
    logclose("...");
    return 0;
}

