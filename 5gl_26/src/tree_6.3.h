#ifndef _TREE_6_3_H
#define _TREE_6_3_H

#include "fs.h"
#include "array.h"

// ---------------------------------------------------------------------------------------
// --------------------------- Public Tree API Ver 6.3 -----------------------------------
// ---------------------------------------------------------------------------------------

typedef struct tnode {
    fs                      groupword;
    Array                   pageslist;
    int                     cnt;
    struct tnode           *left;
    struct tnode           *right;
} tnode;

extern int                  tree_print(const tnode* root);
extern tnode               *tree_add(tnode *restrict root, fs *restrict str, int pagenum);   // fs * because have to destroy original fs
extern void                 tree_free(tnode *t);

#endif /* !_TREE_6_3_H */
