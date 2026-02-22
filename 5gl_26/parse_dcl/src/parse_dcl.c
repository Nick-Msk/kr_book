#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "log.h"
#include "common.h"
#include "checker.h"
#include "bool.h"
#include "fs.h"
#include "dcl.h"
#include "buffer.h"

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
                case 'f':
                    ke->filename = (char *) argv[0] + 1;        // save pointer
                    logauto(ke->filename);
                    logauto(strlen(argv[0]));
                    argv[0] += strlen(argv[0]) - 1;
                    params++;
                break;
                default:    // probaly it's possible to ignore unknows parameters
                    fprintf(stderr, "Illegal option [%c]\n", c);
                    return logerr(-1, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d", params, argc);
}

// parser, f must be opened for read
static int        parse(FILE *f){
    Token   t  = {.value = fsinit(100)};
    int     cnt = 0;
    fs      out = fsinit(100);
    buffer_set(f);

    fs      datatype = fsinit(100);
    while (gettoken(&t) != TOKEOF){
        fscpy(datatype, t.value);  // just copy from one fs to another
        //fsclone(t.value); // This is create the new fs!
        fsend(out, 0);
        dcl(&out, &t);
        if (t.typ != '\n')
            fprintf(stderr, "SYntax error\n");
        printf("%s: %s %s\n",  fsstr(t.value), fsstr(out), fsstr(datatype) );
    }
    fsfreeall(&datatype, &t.value, &out);
    return cnt; // empty for now
}

const char *usage_str = "Usage: %s -ffilename\n";

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
        printf("%s KR parse task p5.12\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    FILE *f = stdin;
    if (ke.filename)
        f = fopen(ke.filename, "r");
    if (!f){
        perror("Unable to open file");     // TODO: inject perror into sysraise
        sysraiseint("Unable to open file %s\n", ke.filename);
    }
    parse(f);

    fclose(f);
    logclose("...");
    return 0;
}

