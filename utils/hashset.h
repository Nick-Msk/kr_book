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

static inline const char            *hset_type_name(hset_type t){
    switch (t){
        CASE_RETURN(HSET_INT);
        CASE_RETURN(HSET_LONG);
        CASE_RETURN(HSET_DBL);
        CASE_RETURN(HSET_FS);
        CASE_RETURN(HSET_PTR);
        default: return "";
    }
}

typedef struct hset_elem {
    union {
        int                 ival;
        long                lval;    // can be ANY type
        double              dval;
        fs                  fsval;  // NOT implemented yet
        void               *pval;
    };
    struct hset_elem   *next;
} hset_elem;

typedef struct hset {
    int             sz;     // init size!
    int             flags;  // empty for now
    hset_elem     **table;
} hset;

#define                 HSET(size, typ) (hset) {.sz = (size), .flags = (typ), .table = 0 }

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

extern hset             hset_init(int sz, hset_type typ);      // #define will be for particular type
extern hset             hset_reinit(hset *se, int newsz);
//
extern void             hset_free(hset *se);
//
extern hset             hset_clone(const hset *se);
extern hset             hset_fromiarr(const int *iarr);  // iarr must be null-ended array
// extern hset             hset_fromfsarr(fs_array iarr);   not supported yet
extern hset             hset_fromlarr(const long *larr);

// -------------------- ACCESS AND MODIFICATORS ------------------------

// ------------------------ Element access -----------------------------
extern bool             hset_iset(hset *se, int val);
extern bool             hset_lset(hset *se, long val);

extern bool             hset_iremove(hset *se, int val);
extern bool             hset_lremove(hset *se, long val);

extern bool             hset_iget(const hset *se, int val);
extern bool             hset_lget(const hset *se, long val);

// ------------------------ PRINTERS/CHECKERS --------------------------

extern int              hset_techfprint(FILE *restrict out, const hset *se, int cnt);
static inline int       hset_techprint(const hset *se, int cnt);

extern bool             hset_validate(FILE *out, const hset *restrict se);

#endif /* !_HASHSET_H */
