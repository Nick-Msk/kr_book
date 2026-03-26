#ifndef _TREE_P6_5_H
#define _TREE_P6_5_H

#include "fs.h"

// ---------------------------------------------------------------------------------
// --------------------------- Public Tree API -------------------------------------
// ---------------------------------------------------------------------------------

typedef struct tnode {
    fs                 word;
    int                cnt;
    struct tnode      *left;
    struct tnode      *right;
} tnode;

extern int               treeprint(const tnode* root);
extern tnode            *treeadd(tnode *restrict root, fs *restrict str);   // fs * because have to destroy original fs
extern void              treefree(tnode *t);

#endif /* !_TREE_P6_5_H */

