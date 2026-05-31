#ifndef _HASHSET_H
#define _HASHSET_H

// ---------------------------------------------------------------------------------
// --------------------------- Public Set API --------------------------------------
// ---------------------------------------------------------------------------------

// --------------- Includes -----------------------

#include <stdio.h>

#include "fs.h"

// ----------- CONSTANTS AND GLOBALS ---------------

// ------------------- TYPES -----------------------

typedef enum hset_type
    { HSET_INT = 1, HSET_LONG, HSET_DBL, HSET_FS, HSET_PTR }
hset_type;

typedef struct hset_elem {
    union {
        int                 val;
        long                lval;    // can be ANY type
        double              dval;
        fs                  fsval;  // NOT implemented yet
        void               *vval;
    };
    struct hset_elem   *next;
} hset_elem;

typedef struct hset {
    int         sz;     // init size!
    int         flags;  // empty for now
    hset_elem  *table;
} hset;

#define                 HSET(size, typ) (hset) {.sz = (size), .flags = (typ), .table = 0 }

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

extern hset             hset_init(int sz, hset_type typ);      // #define will be for particular type
extern hset             hset_reinit(hset *s, int newsz);
//
extern void             hset_free(hset *s);
//
extern hset             hset_clone(const hset *s);
extern hset             hset_fromiarr(const int *iarr);  // iarr must be null-ended array
// extern hset             hset_fromfsarr(fs_array iarr);   not supported yet
extern hset             hset_fromlarr(const long *larr);

// -------------------- ACCESS AND MODIFICATORS ------------------------

// ------------------------ Element access -----------------------------
extern bool             hset_iset(hset *s, int val);
extern bool             hset_lset(hset *s, long val);

extern bool             hset_iremove(hset *s, int val);
extern bool             hset_lremove(hset *s, long val);

extern bool             hset_iget(const hset *s, int val);
extern bool             hset_lget(const hset *s, long val);

// ------------------------ PRINTERS/CHECKERS --------------------------

extern int              hset_techfprint(const FILE *restrict out, const hset *s, int cnt);
static inline int       hset_techprint(const hset *s, int cnt);

#endif /* !_HASHSET_H */
