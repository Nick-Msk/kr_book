#include <stdio.h>
#include <stdlib.h>

#include "error.h"
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

tnode                  *treeadd(tnode *restrict root, fs *restrict str){   // fs * because have to destroy original fs
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

