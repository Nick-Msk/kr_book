#include <stdio.h>
#include <string.h>

#include "getword.h"
#include "log.h"
#include "fs.h"
#include "tree_6.4.h"
#include "word_tree.h"
#include "buffer.h"

typedef struct Keys {
    bool        version;
    bool        tolower;
    bool        reverse;
    char       *filename;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){ .version = false, .tolower = false, .filename = 0, .reverse = false, __VA_ARGS__}

static int              parse_keys(const char *argv[], Keys *ke){
    logenter("...");
    int     argc = 1, params = 0;
    if (!ke)
        return userraiseint(-1, "Zero ke!!! Error!");   // raise here
    char    c;
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
                        logauto(ke->tolower = true);
                        params++;
                    }
                break;
                case 'r':
                    if (!ke->reverse){
                        ke->reverse = true;
                        params++;
                    }
                break;
                case 'f':
                    ke->filename = (char *) argv[1];        // save pointer
                    argv++; // next argv is filename
                    argv[0] += strlen(argv[0]) - 1; // shift
                    params++;
                break;
                default:    // probaly it's possible to ignore unknows parameters
                    fprintf(stderr, "Illegal option [%c]\n", c);
                    return logerr(-1, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d", params, argc);
}

static const char   *usage_str = "Usage: %s -v -f -i -r\n";

static count_node       *cnode_tree_iterall(const wordcntnode *restrict root, count_node *restrict orig);

int                      main(int argc, const char *argv[]){
    logsimpleinit("Start");

    Keys    ke = Keysinit();
    argc = parse_keys(argv, &ke);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s KR cross link 6.3\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    FILE       *in = stdin;
    if (ke.filename){
        if ( (in = fopen(ke.filename, "r")) == 0)
            userraiseint(ERR_UNABLE_OPEN_FILE, "Unable to open file %s", ke.filename);
    }
    buffer_set(in);         // file or stdin

    wordcntnode *root = 0;
    fs           word = FS();   // init empty with fsalloc

    while ( !fsisempty(word = getword(word, ke.tolower, false, false) ) ) { // refactored to apply new parameters of getword()
        if (isalpha_u(*fsstr(word) ) )
            root = wcnt_tree_add(root, &word);
    }

    printf("Total: %d\n", wcnt_tree_print(root) );

    count_node      *cntr = cnode_tree_iterall(root, 0);   // convert

    wcnt_tree_free(root);

    printf("Total: %d\n", cnode_tree_print(cntr, ke.reverse) );    // print as counter
    cnode_tree_free(cntr);

    if (in != stdin)
        fclose(in);

    fsfree(word);

    if (!fs_alloc_check(false))
        logmsg("Warning: incorrect allocation of fs's");

    return logret(0, "end...");  // as replace of logclose()
}

// should be creared via macros
static count_node       *cnode_tree_iterall(const wordcntnode *restrict orig, count_node *restrict root){
    if (orig){
        root = cnode_tree_iterall(orig->left, root);
        root = cnode_tree_add(root, orig->cnt, &orig->word); /* ACTION HERE */
        root = cnode_tree_iterall(orig->right, root);
    }
    return root;
}

