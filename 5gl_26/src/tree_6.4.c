#include <stdio.h>

#include "error.h"
#include "fs.h"
#include "fs_array.h"
#include "tree_6.4.h"

static const int                G_FSARR_INITCAP = 10;

// ----------------------------------- Utility ----------------------------------------------
static void                     cnode_tree_freenode(count_node *node){
    fsarrfree(node->strs);
    free(node);
}

static int                      printfsarray(fsarray lst, int tot){
    TODO:
}

// node != 0
static int                      cnode_tree_prinwordcntnode(const count_node* node){
    int cnt = printf("%d:", node->value);
    cnt += printfsarray(node->strs, node->cnt);
    return cnt;
}

static inline count_node      *cnode_tree_alloc(void){
    return (count_node *) malloc(sizeof(count_node) );
}

static count_node              *cnode_tree_createnode(int value, const fs *str){
    count_node *node = cnode_tree_alloc();
    if (!node)
        userraiseint(ERR_UNABLE_ALLOCATE, "%zu bytes", sizeof(count_node) );
    node->value = value;
    node->strs = fsarr_init(G_FSARR_INITCAP);   // will raise if not allocated
    node->cnt = 0;      // iterator
    node->strs.ar[node->cnt++] = fs_clone(str);
    node->left = node->right = 0;
    return node;
}

// ----------------------------------- API --------------------------------------------------
void                            cnode_tree_free(count_node *node){
    if (node){
        cnode_tree_free(node->left);
        cnode_tree_free(node->right);
        cnode_tree_freenode(node);
    }
}

count_node                     *cnode_tree_add(count_node *restrict node, int value, const fs *restrict str){
    int     cond;
    if (!node)
        node = cnode_tree_createnode(value, str);
    else if ( (cond = (value == node->value) ) ){ // find + attach
        if (node->cnt >= node->strs.cnt)
            fsarr_increaseby(&node->strs, G_FSARR_INITCAP);    // increase
        node->strs.ar[node->cnt++] = fs_clone(str);;
    }
    else if (cond < 0)
        node->left = cnode_tree_add(node->left, value, str);
    else    // cond > 0
        node->right = cnode_tree_add(node->right, value, str);
    return node;
}

int                             cnode_tree_print(const count_node* node, bool reverse){
    int         cnt = 0;
    if (node){
        count_node *t = reverse ? node->right : node->left;
        cnt += cnode_tree_print(t, reverse);
        cnode_tree_prinwordcntnode(node), cnt++;
        t = reverse ? node->left : node->right;
        cnt += cnode_tree_print(t, reverse);
    }
    return cnt;
}

