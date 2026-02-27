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
#include "fileutils.h"

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
        //logauto(*argv);
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

// TODO: move to fileutils.c
int                     print_file(FILE *f){
    rewind(f);
    fs s = readfs_file(f);
    int cnt = printf("%s", fsstr(s) );    // TODO: why don't use fs printer?
    fsfree(s);
    return cnt;
}

const char *usage_str = "Usage: %s -ffilename\n";

int                     main(int argc, const char *argv[]){
    logsimpleinit("Start");

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
    buffer_set(f);

    if (!errsethandler())   // WHAT THAT FOR???? it this is required that should be in try {} block
         return logerr(2, "Unable to setup handler");

    if (!try() ){
        parse();
        print_file(f);
    } else {
        logmsg("Error while parsing");
        err_fprintstacktrace(stderr);
    }
    fclose(f);
    logclose("...");
    return 0;
}

