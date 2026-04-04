#include <stdio.h>

#include "error.h"
#include "fs.h"
#include "array.h"
#include "tree_6.3.h"

static const int        G_TREE_FSARR_INITCAP = 40;

static int              printfslist(Array a, int tot){
    int cnt = 0;
    for (int i = 0; i < tot / 2; i ++){
        cnt += printf("%4d", a.iv[2 * i]);
        if (a.iv[2 * i + 1] > 0)
            cnt += printf("/%2d",  a.iv[2 * i + 1]);
        cnt += printf("\t");
    }
    cnt += printf("\n");
    return cnt;
}


// simple printer
static inline int       tree_printnode(const tnode* node){
    int cnt = 0;
    if (node){
        cnt = printf("[%s] pages: ", node->groupword.v );
        cnt += printfslist(node->pageslist, node->cnt);
    }
    return cnt;
}

// simple free
static void              tree_freenode(tnode *node){
    Arrayfree(node->pageslist);
    fsfree(node->groupword);
    free(node);
}

static inline tnode     *tree_alloc(void){
    return (tnode *) malloc(sizeof(tnode));
}

static tnode            *tree_createnode(const fs *restrict str, int pagenum){
    tnode *root;
    if (! (root = tree_alloc() ) )
        userraiseint(ERR_UNABLE_ALLOCATE, "%zu bytes", sizeof(tnode) );

    root->groupword = fs_clone(str); // 1 copy per group!
    root->pageslist = IArray_create(G_TREE_FSARR_INITCAP, ARRAY_ZERO);  // will raise if unable to allocate
    root->pageslist.iv[0] = pagenum;
    root->cnt = 2;
    root->pageslist.iv[root->cnt - 1] = 1;    // 1 - init
    root->left = root->right = 0;
    return root;
}


// API

tnode                  *tree_add(tnode *restrict root, fs *restrict str, int pagenum){   // fs * because have to destroy original fs (probably after using fs_move() )
    int     cond;
    if (!root)
        root = tree_createnode(str, pagenum);
    else if ( (cond = fs_cmp(str, &root->groupword) ) == 0) {
            if (pagenum > root->pageslist.iv[root->cnt - 2] ){  // if next pagenum
                if (root->cnt >= Arraylen(root->pageslist) )
                        root->pageslist = Array_increase(root->pageslist, root->cnt);   // exception can be here
                logsimple("%s: root->cnt = %d, pagenum = %d", fsstr(root->groupword), root->cnt, pagenum);
                root->pageslist.iv[root->cnt] = pagenum;   // setup next pagenum and shift cnt
                root->cnt += 2;
            }
            root->pageslist.iv[root->cnt - 1]++;      // count of this word in this pagenum!
         }
    else if (cond < 0)
        root->left = tree_add(root->left, str, pagenum);
    else    // cond > 0
        root->right = tree_add(root->right, str, pagenum);
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
