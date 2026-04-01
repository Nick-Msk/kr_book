#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "fs.h"
#include "tree_6.2.h"

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
    root->words
    root->words[root->cnt++] = fs_clone(str);
    root->cnt = 1;
    root->left = root->right = 0;
    return root;
}

tnode                  *tree_add(tnode *restrict root, fs *restrict str, int length){   // fs * because have to destroy original fs (probably after using fs_move() )
    int     cond;
    if (!root)
        root = tree_createnode(str);
    else {
        fs_substr(str, 1, length)           // TODO: think if fsclone() is required
        if ( (cond = fscmp(*str, root->word) ) == 0) // find + attach
            root->cnt++;
        else if (cond < 0)
            root->left = tree_add(root->left, str);
        else    // cond > 0
            root->right = tree_add(root->right, str);
    }
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


