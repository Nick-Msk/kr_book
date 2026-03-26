#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "fs.h"
#include "tree_p6.5.h"

// simple printer
static inline int       treeprintnode(const tnode* str){
    const char *s = 0;
    int cnt = 0;
    if (str)
        s = str->word.v, cnt = str->cnt;
    return printf("%4d:[%s]\n", cnt, s );
}

// simple free
static void              treefreenode(tnode *node){
    fsfree(node->word);
    free(node);
}

static inline tnode     *treealloc(void){
    return (tnode *) malloc(sizeof(tnode));
}


static tnode            *treecreatenode(fs *str){
    tnode *root;
    if (! (root = treealloc() ) )
        userraiseint(ERR_UNABLE_ALLOCATE, "%zu bytes", sizeof(tnode) );
    // root->word = fs_move(str); TEST THAT, but simple copy for now
    root->word = fsclone(*str);
    root->cnt = 1;
    root->left = root->right = 0;
    return root;
}

tnode                  *treeadd(tnode *restrict root, fs *restrict str){   // fs * because have to destroy original fs (probably after using fs_move() )
    int     cond;
    if (!root)
        root = treecreatenode(str);
    else if ( (cond = fscmp(*str, root->word) ) == 0) // find + attach
        root->cnt++;
    else if (cond < 0)
        root->left = treeadd(root->left, str);
    else    // cond > 0
        root->right = treeadd(root->right, str);
    return root;
}

void                    treefree(tnode *node){
    if (node){
        treefree(node->left);
        treefree(node->right);
        treefreenode(node);
    }
}

// tree API
int                     treeprint(const tnode* node){
    int         cnt = 0;
    if (node){
        cnt += treeprint(node->left);
        treeprintnode(node), cnt++;
        cnt += treeprint(node->right);
    }
    return cnt;
}

// -------------------------------------- Int tree linked fs ----------------------------------------------------

// now only 1 element
static int                          printfslist(const fs *str){
    int cnt = 0;
    if (str)    // iteration must be here
        cnt += printf("[%s]\n", str->v );
    return cnt;
}

// now only 1 fs
static void                         freefslist(fs *str){
    fs_free(str);   // only alloc will be freed
}

// free only current node
static void                         inttreefreenode(inttree_linkedfs *root){
    if (root->words)
        freefslist(root->words);
    free(root);
}


static inline inttree_linkedfs     *inttreealloc(void){
    return (inttree_linkedfs *) malloc(sizeof(inttree_linkedfs));
}

static inttree_linkedfs            *inttreecreatenode(int value, const fs *str){
    inttree_linkedfs *root = inttreealloc();
    if (! root)
        userraiseint(ERR_UNABLE_ALLOCATE, "%zu bytes", sizeof(inttree_linkedfs) );
    root->value = value;
    root->cnt = 1;
    // TODO: ref here!!!!!
    root->words = malloc(sizeof(fs) );
    if (!root->words)
        userraiseint(ERR_UNABLE_ALLOCATE, "%zu bytes", sizeof(fs) );
    *root->words = fsclone(*str); // should be list of fs here
    return root;
}

// -------------------- Public API initree -----------------------------------

int                                 intttreeprintnode(const inttree_linkedfs* node){
    int cnt = 0;
    if (node){
        cnt += printf("Value %d, sz %d, first of string: ", node->value, node->cnt);
        cnt += printfslist(node->words);
    }
    return cnt;
}

void                                intttreefree(inttree_linkedfs *root){
    if (root){
        intttreefree(root->left);
        intttreefree(root->right);
        inttreefreenode(root);
    }
}
inttree_linkedfs                   *inttreeadd(inttree_linkedfs *restrict root, const fs *restrict str){
    int     cond;
    int     value = str->len;
    if (!root)
        root = inttreecreatenode(value, str);
    else if ( (cond = (value - root->value) ) == 0) // find + attach
        root->cnt++;
    else if (cond < 0)
        root->left = inttreeadd(root->left, str);
    else    // cond > 0
        root->right = inttreeadd(root->right, str);
    return root;
}

int                                 intttreeprint(const inttree_linkedfs* node){
    int     cnt = 0;
    if (node){
        cnt += intttreeprint(node->left);
        intttreeprintnode(node), cnt++;
        cnt += intttreeprint(node->right);
    }
    return cnt;
}

