#include <stdio.h>

#include "error.h"
#include "fs.h"
#include "word_tree.h"

// simple printer
static inline int       tree_prinwordcntnode(const wordcntnode* str){
    const char *s = 0;
    int cnt = 0;
    if (str)
        s = str->word.v, cnt = str->cnt;
    return printf("%4d:[%s]\n", cnt, s );
}

// simple free
static void              tree_freenode(wordcntnode *node){
    fsfree(node->word);
    free(node);
}

static inline wordcntnode     *tree_alloc(void){
    return (wordcntnode *) malloc(sizeof(wordcntnode));
}


static wordcntnode            *tree_createnode(fs *str){
    wordcntnode *root;
    if (! (root = tree_alloc() ) )
        userraiseint(ERR_UNABLE_ALLOCATE, "%zu bytes", sizeof(wordcntnode) );
    // root->word = fs_move(str); TEST THAT, but simple copy for now
    root->word = fsclone(*str);
    root->cnt = 1;
    root->left = root->right = 0;
    return root;
}

wordcntnode                  *wcnt_tree_add(wordcntnode *restrict root, fs *restrict str){   // fs * because have to destroy original fs (probably after using fs_move() )
    int     cond;
    if (!root)
        root = tree_createnode(str);
    else if ( (cond = fscmp(*str, root->word) ) == 0) // find + attach
        root->cnt++;
    else if (cond < 0)
        root->left = wcnt_tree_add(root->left, str);
    else    // cond > 0
        root->right = wcnt_tree_add(root->right, str);
    return root;
}

void                    wcnt_tree_free(wordcntnode *node){
    if (node){
        wcnt_tree_free(node->left);
        wcnt_tree_free(node->right);
        tree_freenode(node);
    }
}

int                     wcnt_tree_print(const wordcntnode* node){
    int         cnt = 0;
    if (node){
        cnt += wcnt_tree_print(node->left);
        tree_prinwordcntnode(node), cnt++;
        cnt += wcnt_tree_print(node->right);
    }
    return cnt;
}

