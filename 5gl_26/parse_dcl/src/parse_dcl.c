#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "log.h"
#include "common.h"
#include "checker.h"
#include "bool.h"
#include "fs.h"


typedef struct Keys {
    const char *    filename;
    bool            version;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){.version = false, .filename = 0, __VA_ARGS__}

static int              parse_keys(const char *argv[], Keys *ke){
    logenter("...");
    int     argc = 1, params = 0;
    if (!ke)
        return logerr(-1, "Zero ke!!! Error!");
    char    c, *pos;
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
                case 'f':
                    ke->lines = strtol(++argv[0], &pos, 10);
                    argv[0] = pos - 1; // -1 to break next while()
                    params++;
                break;
                default:    // probaly it's possible to ignore unknows parameters
                    fprintf(stderr, "Illegal option [%c]\n", c); 
                    return logerr(-1, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d", params, argc);
}


const char *usage_str = "Usage: %s -ffilename\n";

int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/"__FILE__".log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    Keys ke = Keysinit(.lines = 10);
    argc = parse_keys(argv, &ke);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s KR parse task p5.13\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    FILE *f = stdin;
    if (ke.filename)
        f = fopen(ke.filename, "r");
    if (!f){
        perror("Unable to open file");     // TODO: inject perror into sysraise
        sysraiseint("Unable to open file %s\n", filename);
    }

    /*typedef struct Token {
        toktype  typ;
        fs       value;
    } Token; */
    Token t;
    fs    datatype = fsinit(100);
    while (gettoken(f, &t) != EOF){
        datatype = fscopy(t.value); // from Token

    }

    logclose("%d", cnt);
    return 0;
}

