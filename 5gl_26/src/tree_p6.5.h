#ifndef _TREE_P6_5_H
#define _TREE_P6_5_H

#include "fs.h"
#include "fs_array.h"

// ---------------------------------------------------------------------------------
// --------------------------- Public Tree API -------------------------------------
// ---------------------------------------------------------------------------------

typedef struct tnode {
    fs                      word;
    int                     cnt;
    struct tnode           *left;
    struct tnode           *right;
} tnode;

extern int                  tree_print(const tnode* root);
extern tnode               *tree_add(tnode *restrict root, fs *restrict str);   // fs * because have to destroy original fs
extern void                 tree_free(tnode *t);

// tree based on counts with a list of values TODO: now only COUNT of fs with 1-st fs
typedef struct inttree_linkedfs {
    int                         value;
    fsarray                     words;
    struct inttree_linkedfs    *left;
    struct inttree_linkedfs    *right;
} inttree_linkedfs;

extern void                 intttree_free(inttree_linkedfs *root);
extern inttree_linkedfs    *inttree_add(inttree_linkedfs *restrict root, const fs *restrict str, int value);
extern int                  intttree_print(const inttree_linkedfs* root);

#endif /* !_TREE_P6_5_H */

