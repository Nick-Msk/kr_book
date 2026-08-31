#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "log.h"
#include "common.h"
#include "fs.h"
#include "fs_iter.h"
#include "error.h"
#include "buffer.h"
#include "tree_p6.5.h"
#include "getword.h"

typedef struct Keys {
    bool    version;
    bool    lower;
    // ...
} Keys;
#define                 Keysinit(...) (Keys){ .version = false, .lower = false, __VA_ARGS__}

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
                case 'l':
                    if (!ke->lower){
                        ke->lower = false;
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

static const char   *usage_str = "Usage: %s -v -i\n";

#define      tree_iter_create(type, origin, action, value) TODO:
#define      tree_iter_create_rev(type, origin, action, value) TODO:
#define      tree_iter(type, origin, action)
#define      tree_iter_rev(type, origin, action)

// should be creared via macros
static inttree_linkedfs              *inttree_iterall(tnode *root, inttree_linkedfs *cntroot){
    if (root){
        cntroot = inttree_iterall(root->left, cntroot);
        cntroot = inttree_add(cntroot, &root->word, root->cnt); /* ACTION HERE */
        cntroot = inttree_iterall(root->right, cntroot);
    }
    return cntroot;
}

int                      main(int argc, const char *argv[]){
    logsimpleinit("Start");

    Keys    ke = Keysinit();
    argc = parse_keys(argv, &ke);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s KR find any keywords p6.5\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    buffer_set(stdin);

    tnode       *root = 0;
    fs           word = FS();   // init empty with fsalloc

    while ( !fsisempty(word = getword(word, ke.lower, false, false) ) ) { // refactored to apply new parameters of getword()
        if (isalpha_u(*fsstr(word) ) )
            root = tree_add(root, &word);
    }

    printf("Total: %d\n", tree_print(root) );

    inttree_linkedfs *cntr = inttree_iterall(root, 0);   // convert

    tree_free(root);

    printf("Total: %d\n", intttree_print(cntr) );    // print as counter
    intttree_free(cntr);

    fsfree(word);   // in case if treeadd will not user fs_move()
    if (!fs_alloc_check(false))
        logmsg("Warning: incorrect allocation of fs's");

    return logret(0, "end...");  // as replace of logclose()
}

