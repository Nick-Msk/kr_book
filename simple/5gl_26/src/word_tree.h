#ifndef _WORD_TREE_H
#define _WORD_TREE_H

#include "fs.h"
#include "fs_array.h"

// --------------------------------------------------------------------------------------
// ------------------- Public Word + Count Tree API -------------------------------------
// --------------------------------------------------------------------------------------

typedef struct wordcntnode {
    fs                      word;
    int                     cnt;
    struct wordcntnode     *left;
    struct wordcntnode     *right;
} wordcntnode;

extern int                  wcnt_tree_print(const wordcntnode* root);
extern wordcntnode         *wcnt_tree_add(wordcntnode *restrict root, fs *restrict str);   // fs * because have to destroy original fs
extern void                 wcnt_tree_free(wordcntnode *t);

#endif /* !_WORD_TREE_H */

