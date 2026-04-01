#ifndef _TREE_6_2_H
#define _TREE_6_2_H

#include "fs.h"
#include "fs_array.h"

// ---------------------------------------------------------------------------------------
// --------------------------- Public Tree API Ver 2 -------------------------------------
// ---------------------------------------------------------------------------------------

typedef struct tnode {
    fsarray                 words;
    int                     cnt;
    struct tnode           *left;
    struct tnode           *right;
} tnode;

extern int                  tree_print(const tnode* root);
extern tnode               *tree_add(tnode *restrict root, fs *restrict str, int length);   // fs * because have to destroy original fs
extern void                 tree_free(tnode *t);

#endif /* !_TREE_6_2_H */

