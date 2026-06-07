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

typedef union hset_value {
        int                 ival;
        long                lval;    // can be ANY type
        double              dval;
        fs                 *fsval;
        void               *pval;
        uint64_t            u64;    // for hash
} hset_value;

_Static_assert(sizeof(hset_value) == sizeof(uint64_t),
               "hset_value must be exactly 8 bytes");

static inline void          hsetval_fprint(FILE *restrict out, const char *restrict msg, hset_value val, hset_type typ){
    if (out){
        if (msg)
            fprintf(out, "%s ", msg);
        switch (typ){
            case HSET_INT:
                fprintf(out, "%d", val.ival);
            break;
            case HSET_LONG:
                fprintf(out, "%ld", val.lval);
            break;
            case HSET_DBL:
                fprintf(out, "%lf", val.dval);
            break;
            case HSET_PTR:
                fprintf(out, "%p", val.pval);
            break;
            case HSET_FS:
                fs_fprint(out, val.fsval, 0);
            break;
        }
    }
}

static inline void          hsetval_log(hset_value val, hset_type typ){
    hsetval_fprint(logfile, 0, val, typ);
}

typedef struct hset_elem {
    hset_value         v;
    struct hset_elem   *next;
} hset_elem;

typedef struct hset {
    int             sz;     // init size!
    int             flags;  // empty for now
    int             count;
    hset_elem     **table;
} hset;

#define                 HSET(size, typ) (hset) {.sz = (size), .flags = (typ), .table = 0 }
#define                 HSET_VALUE          (hset_value) {.u64 = 0 }
#define                 HSET_INTVALUE(val)  (hset_value) {.u64 = 0, .ival = val }
#define                 HSET_LONGVALUE(val) (hset_value) {.u64 = 0, .lval = val }
#define                 HSET_DBLVALUE(val)  (hset_value) {.u64 = 0, .dval = val }
#define                 HSET_PTRVALUE(val)  (hset_value) {.u64 = 0, .pval = val }
#define                 HSET_FSVALUE(val)   (hset_value) {.fsval = val }
// create value from pointer
static inline hset_value        hset_createval(const void *p, hset_type typ){
    hset_value tmp = HSET_VALUE;  // init
    switch (typ){
        case HSET_INT:
            tmp.ival = *(const int *) p;
        break;
        case HSET_LONG:
            tmp.lval = *(const long *) p;
        break;
        case HSET_DBL:
            tmp.dval = *(const double *) p;
        break;
        case HSET_PTR:
            tmp.pval = *(void * const *) p;
        break;
        case HSET_FS:  // NOT SURE, PROBABLY DEEP COPY IS REQUIRED
            tmp.fsval = *(fs * const *) p;
        break;
        default:
            userraiseint(ERR_UNSUPPORTED_TYPE, "type %d isn't suppoted", typ);
        break;
    }
    return tmp;
}

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

extern hset             hset_init(int sz, hset_type typ);      // #define will be for particular type
extern hset             hset_init_resize(hset *se, int newsz);       // TODO:
//
extern void             hset_free(hset *se);
//
extern hset             hset_clone(const hset *se);

extern hset             hset_cloneas(const hset *se, hset_type typ);    // TODO:

// universale loader
extern hset             hset_fromanyarr(const void *arr, int sz, hset_type typ);
// typed
static inline hset      hset_fromiarr(const int *iarr, int sz){
    return hset_fromanyarr(iarr, sz, HSET_INT);
}
// not SURE TODO:
static inline hset      hset_fromfsarr(/*fs_array iarr*/ const fs *fsarr, int sz){
    return hset_fromanyarr(fsarr, sz, HSET_FS);
}
static inline hset      hset_fromlarr(const long *larr, int sz){
    return hset_fromanyarr(larr, sz, HSET_LONG);
}
static inline hset      hset_fromdarr(const double *darr, int sz){
    return hset_fromanyarr(darr, sz, HSET_DBL);
}
extern hset             hset_fromparr(const void **parr, int sz){
    return hset_fromanyarr(parr, sz, HSET_PTR);
}
// just intersect with construct
extern hset             hset_init_intersect(const hset *restrict se1, const hset *restrict se2);
// minus with construct with construct
extern hset             hset_init_minus(const hset *restrict se1, const hset *restrict se2);
// simm diff with construct
extern hset             hset_init_symmdiff(const hset *restrict a, const hset *restrict b);

// -------------------- ACCESS AND MODIFICATORS ------------------------

// ------------------------ Element access -----------------------------

extern bool             hset_set(hset *se, hset_value val);
// try to delete elemenet, true if deleted, false if not found
extern bool             hset_del(hset *se, hset_value val);

extern bool             hset_get(const hset *se, hset_value val);

static inline int       hset_cnt(const hset *se){
    return se->count;
}

extern void             hset_clean(hset *se);
// origin wll be cleaned
static inline hset     *hset_move(hset *target, hset * origin){
    hset_free(target);
    *target = *origin;
    origin->table = 0;
    origin->flags = origin->sz = 0;
    return logsimpleret(target, "moved to %p, sz %d, cnt %d", target, target->sz, target->count);
}

extern bool             hset_eq(const hset *restrict se1, const hset *restrict se2);

extern bool             hset_noteq(const hset *restrict se1, const hset *restrict se2);
// load values from array, any type
extern int              hset_loadanyarr(hset *restrict se, const void *arr, int sz, hset_type typ);

static inline int       hset_loadiarr(hset *restrict se, const int *iarr, int sz){
    return hset_loadanyarr(se, iarr, sz, HSET_INT);
}
static inline int       hset_loadlarr(hset *restrict se, const long *larr, int sz){
    return hset_loadanyarr(se, larr, sz, HSET_LONG);
}
static inline int       hset_loaddarr(hset *restrict se, const double *darr, int sz){
    return hset_loadanyarr(se, darr, sz, HSET_DBL);
}
static inline int       hset_loadparr(hset *restrict se, const void * const *restrict parr, int sz){
    return hset_loadanyarr(se, parr, sz, HSET_PTR);
}
// TODO: ???
static inline int       hset_loadfsarr(hset *restrict se, const fs *restrict fsarr, int sz){
    return hset_loadanyarr(se, fsarr, sz, HSET_FS);
}
// check if all of se2 in se1 strictly or not
extern bool             hset_subset_check(const hset *restrict se1, const hset *restrict se2, bool strict);
// check if all of se2 in se1
static inline bool      hset_in(const hset *restrict se1, const hset *restrict se2){
    return hset_subset_check(se1, se2, false);
}
// check if all of se2 in se1  but se2 not equal se1
static inline bool      hset_strictin(const hset *restrict se1, const hset *restrict se2){
    return hset_subset_check(se1, se2, true);
}
// se1 -= se2 as SET
extern int              hset_minus(hset *restrict se1, const hset *restrict se2);
// se1 insersect= se2 as SET
extern int              hset_intersect(hset *restrict se1, const hset *restrict se2);
// se1 symmdiff= se2 as SET
extern hset            *hset_symmdiff(hset *restrict a, const hset *restrict b);
// ------------------------ PRINTERS/CHECKERS --------------------------

extern int              hset_techfprint(FILE *restrict out, const hset *se, int cnt);
static inline int       hset_techprint(const hset *se, int cnt){
    return hset_techfprint(stdout, se, cnt);
}

extern bool             hset_validate(FILE *out, const hset *restrict se);

#endif /* !_HASHSET_H */

