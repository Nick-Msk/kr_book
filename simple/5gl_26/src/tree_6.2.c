#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "fs.h"
#include "tree_6.2.h"

static const int        G_TREE_FSARR_INITCAP = 10;

static int              printfslist(fsarray words, int cntr){
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


// simple printer
static inline int       tree_printnode(const tnode* node){
    int cnt = 0;
    if (node){
        cnt = printf("Group [%s]:%d", node->groupword.v, node->cnt);
        cnt += printfslist(node->words, node->cnt);
    }
    return cnt;
}

// simple free
static void              tree_freenode(tnode *node){
    fsarrfree(node->words);
    fsfree(node->groupword);
    free(node);
}

static inline tnode     *tree_alloc(void){
    return (tnode *) malloc(sizeof(tnode));
}


static tnode            *tree_createnode(const fs *restrict str, int length){
    tnode *root;
    if (! (root = tree_alloc() ) )
        userraiseint(ERR_UNABLE_ALLOCATE, "%zu bytes", sizeof(tnode) );

    root->groupword = fs_newsubstr(str, 0, length); // 1 copy per group!
    root->cnt = 0;
    root->words = fsarr_init(G_TREE_FSARR_INITCAP);   // will raise if not allocated
    root->words.ar[root->cnt++] = fs_clone(str);
    root->left = root->right = 0;
    return root;
}

tnode                  *tree_add(tnode *restrict root, fs *restrict str, int length){   // fs * because have to destroy original fs (probably after using fs_move() )
    int     cond;
    if (!root)
        root = tree_createnode(str, length);
    else if ( (cond = fs_ncmp(str, &root->groupword, length) ) == 0) { // find + attach
            if (root->cnt == root->words.cnt)
               fsarr_increaseby(&root->words, G_TREE_FSARR_INITCAP);
            root->words.ar[root->cnt++] = fs_clone(str);  // new!!! fs
         }
    else if (cond < 0)
        root->left = tree_add(root->left, str, length);
    else    // cond > 0
        root->right = tree_add(root->right, str, length);
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


