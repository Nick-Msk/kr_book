#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "getword.h"
#include "log.h"
#include "fs.h"
#include "tree_6.2.h"
#include "buffer.h"

typedef struct Keys {
    bool        version;
    bool        tolower;
    char       *filename;
    int         length;
    // ...
} Keys;
#define                 Keysinit(...) (Keys){ .version = false, .tolower = false, .filename = 0, .length = 6, __VA_ARGS__}

static int              parse_keys(const char *argv[], Keys *ke){
    logenter("...");
    int     argc = 1, params = 0;
    if (!ke)
        return userraiseint(-1, "Zero ke!!! Error!");   // raise here
    char    c, *pos;;
    while (*++argv != 0 && **argv == '-'){
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
                    if (!ke->tolower){
                        ke->tolower = true;
                        params++;
                    }
                case 'f':
                    ke->filename = (char *) argv[0] + 1;        // save pointer
                    argv[0] += strlen(argv[0]) - 1;
                    params++;
                break;
                case 'l':
                    ke->length = strtol(++argv[0], &pos, 10);
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

static const char   *usage_str = "Usage: %s -v -f -i -l\n";

int                      main(int argc, const char *argv[]){
    logsimpleinit("Start");

    Keys    ke = Keysinit();
    argc = parse_keys(argv, &ke);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s KR var grouping 6.2\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    FILE       *in = stdin;
    if (ke.filename){
        if ( (in = fopen(ke.filename, "r")) == 0)
            userraiseint(ERR_UNABLE_OPEN_FILE, "Unable to open file %s", ke.filename);
    }
    buffer_set(in);         // file or stdin

    tnode       *root = 0;
    fs           word = FS();   // init empty with fsalloc

    while ( !fsisempty(word = getword(word, ke.tolower, false, false) ) ) {       // false means without comment, line and so on
        if (isalpha_u(*fsstr(word) ) )
            root = tree_add(root, &word, ke.length);        // grouping by length
    }
    if (in != stdin)
        fclose(in);

    tree_print(root);

    tree_free(root);
    fsfree(word);

    if (!fs_alloc_check(false))
        logmsg("Warning: incorrect allocation of fs's");

    return logret(0, "end...");  // as replace of logclose()
}


