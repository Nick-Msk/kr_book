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

typedef struct Keys {
    bool    version;
    bool    sens;
    // ...
} Keys;
#define                 Keysinit(...) (Keys){ .version = false, .sens = true, __VA_ARGS__}

typedef struct tnode {
    fs                 word;
    int                cnt;
    struct tnode      *left;
    struct tnode      *right;
} tnode;

static int              parse_keys(const char *argv[], Keys *ke){
    logenter("...");
    int     argc = 1, params = 0;
    if (!ke)
        return userraiseint(-1, "Zero ke!!! Error!");   // raise here
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
                case 'i':
                    if (!ke->sens){
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
// tree API
static int               treeprint(const tnode* str);
static tnode            *treeadd(tnode *t, fs  *str);   // fs * because have to destroy original fs
static void              treefree(tnode *t);

static const char   *usage_str = "Usage: %s -v -i\n";

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
        logauto(fsstr(word));
        if (isalpha_u(*fsstr(word) ) )
            root = treeadd(root, &word);
    }

    printf("%d\n", treeprint(root) );
    treefree(root);
    fsfree(word);   // in case if treeadd will not user fs_move()

    return logret(0, "end...");  // as replace of logclose()
}

static inline int cupper(int c, bool sens){
    return sens ? c: tolower(c);
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
        elemnext(iter) = cupper(c, sens);
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
            elemnext(iter) = cupper(c, sens);
    }
    elemend(iter);

    return logret(str, "%d - [%s]", str.len, str.v); // that is probably new str
}

// simple printer, str MUST be != 0
static inline int       treeprintnode(const tnode* str){
    return printf("%4d:[%s]\n", str->cnt, str->word.v );
}

// simple free
static void              treefreenode(tnode *node){
    fsfree(node->word);
    free(node);
}

static inline tnode     *treealloc(void){
    return (tnode *) malloc(sizeof(tnode));
}

// tree API
static int               treeprint(const tnode* node){
    int         cnt = 0;
    if (node){
        cnt += treeprint(node->left);
        treeprintnode(node), cnt++;
        cnt += treeprint(node->right);
    }
    return cnt;
}

static tnode            *treeadd(tnode *root, fs  *str){   // fs * because have to destroy original fs
    // TODO:
    int     cond;
    if (!root){
        if (! (root = treealloc() ) )
            userraiseint(ERR_UNABLE_ALLOCATE, "%zu bytes", sizeof(tnode) );
        // root->word = fs_move(str); TEST THAT, but simple copy for now
        root->word = fsclone(*str);
        root->cnt = 1;
        root->left = root->right = 0;
    } else if ( (cond = fscmp(*str, root->word) ) == 0) // find + attach
        root->cnt++;
    else if (cond < 0)
        root->left = treeadd(root->left, str);
    else    // cond > 0
        root->right = treeadd(root->right, str);
    return root;
}

static void              treefree(tnode *node){
    if (node){
        treefree(node->left);
        treefree(node->right);
        treefreenode(node);
    }
}

