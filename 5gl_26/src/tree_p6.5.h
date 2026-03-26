#ifndef _TREE_P6_5_H
#define _TREE_P6_5_H

#include "fs.h"

// ---------------------------------------------------------------------------------
// --------------------------- Public Tree API -------------------------------------
// ---------------------------------------------------------------------------------

typedef struct tnode {
    fs                      word;
    int                     cnt;
    struct tnode           *left;
    struct tnode           *right;
} tnode;

extern int                  treeprint(const tnode* root);
extern tnode               *treeadd(tnode *restrict root, fs *restrict str);   // fs * because have to destroy original fs
extern void                 treefree(tnode *t);

// tree based on counts with a list of values TODO: now only COUNT of fs with 1-st fs
typedef struct inttree_linkedfs {
    int                         value;
    int                         cnt; // count of list
    fs                         *words;  // now ONLY 1 fs
    struct inttree_linkedfs    *left;
    struct inttree_linkedfs    *right;
} inttree_linkedfs;

extern void                 intttreefree(inttree_linkedfs *root);
extern inttree_linkedfs    *inttreeadd(inttree_linkedfs *restrict root, const fs *restrict str, int value);
extern int                  intttreeprint(const inttree_linkedfs* root);

#endif /* !_TREE_P6_5_H */

