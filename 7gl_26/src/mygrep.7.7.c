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
    bool            printfilename;
    //const char     *filename;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){.except = false, .line = false, .version = false, /* .filename = 0, */ .lowcomparation = false, .printfilename = false, __VA_ARGS__}

static                  int parse_keys(const char *argv[], Keys *ke){

    invraise(ke, "Zero ke!!! Error!"); // raise here

    logenter("...");
    int     argc = 1, params = 0;
    char    c;
    while (*++argv != 0 && **argv == '-'){

        //const char *ptr;
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
                    ke->printfilename = true;
                    params++;
                break;
                /* case 'f': 
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
                break; */
                default:    // probaly it's possible to ignore unknows parameters
                    return userraise(-1, ERR_WRONG_PARAMETER, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d", params, argc);
}

const char *usage_str = "Usage: %s -v -l -n -f -x <pattert:str>\n";

int                     main(int argc, const char *argv[]){
    logsimpleinit("Start");

    Keys ke = Keysinit();
    argc = parse_keys(argv, &ke);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s KR: mygrep 7.7\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    if (!argv[argc])
        return userraise(2, ERR_NOT_ENOGH_VALUES, "Empty pattern");

    fs                      pt = fsliteral(argv[argc++]);
    const char             *filename = argv[argc++];  // next elem(s)
    int                     found = 0;
    int                     lineno = 0;
    fs                      s = FS();
    FILE                   *in = stdin;

    do {
        if (filename){ // if non-enpty - then open a file
            if ( (in = fopen(filename, "r") ) == 0)
                return userraise(3, ERR_UNABLE_OPEN_FILE_READ, "Unable to open file %s", filename);
        } else
            logmsg("Empty filename list - use stdin");
        while (fgetslim_fs(in, &s) >= 0){ // zero length lines are ok!
            lineno++;
            int res = ke.lowcomparation ? fs_iinstr(&s, &pt) : fs_instr(&s, &pt);
            if ( (res >= 0) != ke.except){   // >= 0 means found instr
                found++;
                if (ke.printfilename && filename)
                    printf("%s: ", filename);
                if (ke.line)
                    printf("%d: ", lineno);
                printf("%s\n", fsstr(s) );
            }
        }
        if (in != stdin)
            fclose(in);
        if (filename)       // next ONLY if current filename is  present
            logauto(filename = argv[argc++]);  // next
    } while (filename != 0);

    printf("\nTotal %d\n", found);
    return logret(0, "found %d", found);
}

