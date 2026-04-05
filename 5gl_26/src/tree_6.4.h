#ifndef _TREE_6_4_H
#define _TREE_6_4_H

#include "fs.h"

// -------------------------------------------------------------------------------------
// -------------------- Public Tree Countbased API -------------------------------------
// -------------------------------------------------------------------------------------


typedef struct count_node {
    int                         value;  // main ordered value!
    // properties
    fsarray                     strs;   // list
    int                         cnt;    // iterator for property list
    // linkage to the lists
    struct count_node          *left;
    struct count_node          *right;
} count_node;

extern void                 intttree_free(count_node *node);
extern count_node          *inttree_add(count_node *restrict root, int value, const fs *restrict str);
extern int                  intttree_print(const count_node* node, bool reverse);

#endif /* !_TREE_6_4_H */

