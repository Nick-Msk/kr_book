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
    bool            except;
    bool            line;
    bool            version;
    bool            lowcomparation;
    const char     *filename;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){.except = false, .line = false, .version = false,.filename = 0, .lowcomparation = false, __VA_ARGS__}

static                  int parse_keys(const char *argv[], Keys *ke){

    invraise(ke, "Zero ke!!! Error!"); // raise here

    logenter("...");
    int     argc = 1, params = 0;
    char    c;
    while (*++argv != 0 && **argv == '-'){

        const char *ptr;
        argc++;
        while ( (c = *++argv[0]) )
            switch (tolower(c)){
                case 'x':
                    ke->except = true;
                    params++;
                break;
                case 'v':
                    ke->version = true;
                    params++;
                break;
                case 'n':
                    ke->line = true;
                    params++;
                break;
                case 'l':
                    ke->lowcomparation = true;
                    params++;
                break;
                case 'f':
                    // TODO: refactor that to make more pretty
                    if (argv[0][1] == '\0') {
                        if (argv[1]){
                            ptr = argv[1];
                            argv++;
                            argc++;
                        } else
                             return userraise(-1, ERR_WRONG_PARAMETER, "-l option without value (must be integer followed), ex '-l123' or '-l 123'");
                    } else  // argv[1] + 1 is pointer to value
                        ptr = argv[0] + 1;
                    ke->filename = ptr;        // save pointer
                    argv[0] += strlen(argv[0]) - 1; // shift
                    params++;
                break;
                default:    // probaly it's possible to ignore unknows parameters
                    return userraise(-1, ERR_WRONG_PARAMETER, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d", params, argc);
}

const char *usage_str = "Usage: %s -v -l -n -x <pattert:str>\n";

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

    //const char              *pt = argv[argc];
    fs                      pt = fsliteral(argv[argc]);
    int                     found = 0;
    int                     lineno = 0;
    fs                      s = FS();
    FILE                   *in = stdin;

    if (ke.filename)
        if ( (in = fopen(ke.filename, "r") ) == 0)
            return userraise(2, ERR_UNABLE_OPEN_FILE_READ, "Unable to open file %s", ke.filename);

    fstechfprint(logfile, pt);
    while (fgetslim_fs(in, &s) >= 0){ // zero length lines are ok!
        lineno++;
        int res = ke.lowcomparation ? fs_iinstr(&s, &pt) : fs_instr(&s, &pt);
        if ( (res >= 0) != ke.except){   // >= 0 means found instr
            found++;
            if (ke.filename)
                printf("%s: ", ke.filename);
            if (ke.line)
                printf("%d: ", lineno);
            printf("%s\n", fsstr(s) );
        }
    }
    /*while (get_line(buf, MaxLine) > 0){
        lineno++;
        if ( (strstr(buf, pt) != 0) != ke.except){
            if (ke.line)
                printf("%d:", lineno);
            printf("%s", buf);
            found++;
        }
    }*/
    if (in != stdin)
        fclose(in);
    printf("\nTotal %d\n", found);
    logclose("found %d", found);
    return 0;
}

