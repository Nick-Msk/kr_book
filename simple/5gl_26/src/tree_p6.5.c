#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "fs.h"
#include "tree_p6.5.h"

static const int        G_FSARR_INITCAP = 10;

// simple printer
static inline int       tree_printnode(const tnode* str){
    const char *s = 0;
    int cnt = 0;
    if (str)
        s = str->word.v, cnt = str->cnt;
    return printf("%4d:[%s]\n", cnt, s );
}

// simple free
static void              tree_freenode(tnode *node){
    fsfree(node->word);
    free(node);
}

static inline tnode     *tree_alloc(void){
    return (tnode *) malloc(sizeof(tnode));
}


static tnode            *tree_createnode(fs *str){
    tnode *root;
    if (! (root = tree_alloc() ) )
        userraiseint(ERR_UNABLE_ALLOCATE, "%zu bytes", sizeof(tnode) );
    // root->word = fs_move(str); TEST THAT, but simple copy for now
    root->word = fsclone(*str);
    root->cnt = 1;
    root->left = root->right = 0;
    return root;
}

tnode                  *tree_add(tnode *restrict root, fs *restrict str){   // fs * because have to destroy original fs (probably after using fs_move() )
    int     cond;
    if (!root)
        root = tree_createnode(str);
    else if ( (cond = fscmp(*str, root->word) ) == 0) // find + attach
        root->cnt++;
    else if (cond < 0)
        root->left = tree_add(root->left, str);
    else    // cond > 0
        root->right = tree_add(root->right, str);
    return root;
}

void                    tree_free(tnode *node){
    if (node){
        tree_free(node->left);
        tree_free(node->right);
        tree_freenode(node);
    }
}

// tree API
int                     tree_print(const tnode* node){
    int         cnt = 0;
    if (node){
        cnt += tree_print(node->left);
        tree_printnode(node), cnt++;
        cnt += tree_print(node->right);
    }
    return cnt;
}

// -------------------------------------- Int tree linked fs ----------------------------------------------------

static int                          printfslist(fsarray words, int cntr){
    int cnt = 0, i;
    for (i = 0; i < cntr && !fsisnull(words.ar[i]); i++){    // probably iterator is required TODO:
        cnt += printf("[%s] ", fsstr(words.ar[i]) ); // fsarr_get(words, i)->v); //fsstr(words->ar[i]) );
        if ( i % 10 == 9)
            cnt += printf("\n");
    }
    if (! (i % 10 == 0) || i == 0)
        cnt += printf("\n");
    return cnt;
}

// now only 1 fs
static void                         freefslist(fsarray *arr){
    fsarr_free(arr);
}

// free only current node
static void                         inttree_freenode(inttree_linkedfs *root){
    freefslist(&root->words);
    free(root);
}


static inline inttree_linkedfs     *inttree_alloc(void){
    return (inttree_linkedfs *) malloc(sizeof(inttree_linkedfs));
}

static inttree_linkedfs            *inttree_createnode(int value, const fs *str){
    inttree_linkedfs *root = inttree_alloc();
    if (! root)
        userraiseint(ERR_UNABLE_ALLOCATE, "%zu bytes", sizeof(inttree_linkedfs) );
    root->value = value;
    root->words = fsarr_init(G_FSARR_INITCAP);   // will raise if not allocated
    root->cnt = 0;
    logsimple("after init cnt = %d, sz = %d, str = %s, VALUE %d", root->words.cnt, root->words.sz, str->v, value);
    root->words.ar[root->cnt++] = fs_clone(str);
    root->left = root->right = 0;
    return root;
}

// -------------------- Public API initree -----------------------------------

int                                 intttree_printnode(const inttree_linkedfs* node){
    int cnt = 0;
    if (node){
        cnt += printf("(%3d/%3d): ", node->value, node->cnt);
        cnt += printfslist(node->words, node->cnt);
    }
    return cnt;
}

void                                intttree_free(inttree_linkedfs *root){
    if (root){
        intttree_free(root->left);
        intttree_free(root->right);
        inttree_freenode(root);
    }
}
inttree_linkedfs                   *inttree_add(inttree_linkedfs *restrict root, const fs *restrict str, int value){
    logenter("adding %s - %d", str->v, value);
    int     cond;
    if (!root)
        root = inttree_createnode(value, str);
    else { logmsg("VALUE %d, root->VALUE %d", value, root->value);
         if ( (cond = (value - root->value) ) == 0){ // find + attach
        if (root->cnt == root->words.cnt){
            logmsg("INCREASE REQ: cnt = %d, sz = %d for str %s value %d", root->cnt, root->words.sz, str->v, value);
            fsarr_increaseby(&root->words, G_FSARR_INITCAP);
        }
        // TODO: iteracor must be here!
        root->words.ar[root->cnt++] = fs_clone(str);  // new!!! fs
    } else if (cond < 0)
        root->left = inttree_add(root->left, str, value);
    else    // cond > 0
        root->right = inttree_add(root->right, str, value);
    }
    return logret(root, "added  %s - %d", str->v, value);
}

int                                 intttree_print(const inttree_linkedfs* node){
    int     cnt = 0;
    if (node){
        cnt += intttree_print(node->left);
        intttree_printnode(node), cnt++;
        cnt += intttree_print(node->right);
    }
    return cnt;
}

