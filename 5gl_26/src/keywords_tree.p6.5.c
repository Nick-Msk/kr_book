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

typedef struct Keys {
    bool    version;
    bool    sens;
    // ...
} Keys;
#define                 Keysinit(...) (Keys){ .version = false, .sens = true, __VA_ARGS__}

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
                    if (ke->sens){
                        ke->sens = false;
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

static fs                getword(fs str, bool sens);

static const char   *usage_str = "Usage: %s -v -i\n";

// should be creared via macros
static inttree_linkedfs              *inttree_iterall(tnode *root, inttree_linkedfs *cntroot){
    if (root){
        cntroot = inttree_iterall(root->left, cntroot);
        cntroot = inttree_add(cntroot, &root->word, root->cnt);
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

    while ( !fsisempty(word = getword(word, ke.sens) ) ) {
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

static inline int       skip_spaces(void){
    int c;
    while (isspace( c = getch() ) )
        ;
    return c;
}

/*
static int               skip_cl(void){
    int             cnt = 0;        // total count of comments
    int             c, c1;
    bool            comment_and_lines = true;

    while (comment_and_lines){

        c = skip_spaces();
        // SKIP comment or literals
        comment_and_lines = false;
        if (c == '/'){
            if ( (c1 = getch() ) == '/'){  // start comment type 1
                cnt++;
                comment_and_lines = true;
                while ( (c1 = getch()) != EOF && c1 != '\n')
                    ;
            } else if (c1 == '*'){  // comment type 2
                cnt++;
                comment_and_lines = true;
                do {
                    while ( (c1 = getch()) != EOF && c1 != '*')
                        ;
                } while ( (c1 = getch() ) != '/' && c1 != EOF);  // end of /* */ /*
            } else  // not a comment! comment_and_lines remains false
                c = c1; // just like ungetch
        } else if (c == '"') {
            comment_and_lines = true;   // line, so setup flag
            while ( (c = getch()) != EOF && c != '"')
                ;
        } else if (c == '\'') {
            comment_and_lines = true;   // anyway!
            while ( (c = getch()) != EOF && c != '\'')
                ;
        }
        logsimple("comment_and_lines %s", bool_str(comment_and_lines));
    }
    return logsimpleret(c, "[%c], comments %d", c, cnt);
}*/


// str must have heap alloc
static fs                getword(fs str, bool sens){

    logenter("sens %s", bool_str(sens) );

    fsclear(str);   // reset
    int              c;
    fsnew            iter = fsinew(&str);

    c = skip_spaces();       // comment and so on are allowed! TODO: probably use flag -c
    if (c != EOF){
        elemnext(iter) = clower(c, sens);
    } else
        elemclear(iter);    // end flag

    if (!isalpha_u(c) ){
        elemend(iter);
        return logret(str, "%c:%d - [%s]", c, str.len, str.v);
    }
    while ( (c = getch()) != EOF){
        if (!isalnum_u(c) ){
            ungetch(c);
            break;
        } else
            elemnext(iter) = clower(c, sens);
    }
    elemend(iter);

    return logret(str, "%d - [%s]", str.len, str.v); // that is probably new str
}

