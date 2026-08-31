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
    bool            line;
    bool            version;
    int             maxlines;
    bool            printfilename;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){.line = false, .version = false, .maxlines = 0, .printfilename = false, __VA_ARGS__}

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
                case 'v':
                    ke->version = true;
                    params++;
                break;
                case 'n':
                    ke->line = true;
                    params++;
                break;
                case 'l':
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
                    ke->maxlines = atoi(ptr);        // save pointer
                    argv[0] += strlen(argv[0]) - 1; // shift
                    params++;
                break;
                case 'f':
                    ke->printfilename = true;
                    params++;
                break;
                default:    // probaly it's possible to ignore unknows parameters
                    return userraise(-1, ERR_WRONG_PARAMETER, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d", params, argc);
}

const char *usage_str = "Usage: %s -v -l<maxlines> -f -n <pattert:str>\n";

int                     main(int argc, const char *argv[]){
    logsimpleinit("Start");

    Keys ke = Keysinit();
    argc = parse_keys(argv, &ke);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s KR: printfiles 7.8\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    const char             *filename = argv[argc++];  // next elem(s)
    int                     lineno = 0;
    fs                      s = FS();
    FILE                   *in = stdin;

    do {
        if (filename){ // if non-enpty - then open a file
            if ( (in = fopen(filename, "r") ) == 0)
                return userraise(3, ERR_UNABLE_OPEN_FILE_READ, "Unable to open file %s", filename);
        } else
            logmsg("Empty filename list - use stdin");

        printf("\n\t\t\t%s\n\n", filename ? filename : "STDIN");
        while ( (ke.maxlines == 0 || lineno < ke.maxlines) &&  fgetslim_fs(in, &s) >= 0){ // zero length lines are ok!
            lineno++;
            if (ke.printfilename && filename)
                printf("%s: ", filename);
            if (ke.line)
                printf("%d: ", lineno);
            printf("%s\n", fsstr(s) );
        }
        lineno = 0;
        if (in != stdin)
            fclose(in);
        if (filename)       // next ONLY if current filename is  present
            logauto(filename = argv[argc++]);  // next
    } while (filename != 0);

    return logret(0, "");
}

