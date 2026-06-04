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
    hset_elem     **table;
} hset;

#define                 HSET(size, typ) (hset) {.sz = (size), .flags = (typ), .table = 0 }
#define                 HSET_INTVALUE(val)  (hset_value) {.u64 = 0, .ival = val }
#define                 HSET_LONGVALUE(val) (hset_value) {.u64 = 0, .lval = val }
#define                 HSET_DBLVALUE(val)  (hset_value) {.u64 = 0, .dval = val }
#define                 HSET_PTRVALUE(val)  (hset_value) {.u64 = 0, .pval = val }
#define                 HSET_FSVALUE(val)   (hset_value) {.fsval = val }

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

extern hset             hset_init(int sz, hset_type typ);      // #define will be for particular type
extern hset             hset_reinit(hset *se, int newsz);
//
extern void             hset_free(hset *se);
//
extern hset             hset_clone(const hset *se);
extern hset             hset_fromiarr(const int *iarr, int sz);  // iarr must be null-ended array
//extern hset             hset_fromfsarr(fs_array iarr);  //  not supported yet
extern hset             hset_fromlarr(const long *larr, int sz);
extern hset             hset_fromsarr(const double *darr, int sz);
extern hset             hset_fromparr(const void **parr, int sz);

// -------------------- ACCESS AND MODIFICATORS ------------------------

// ------------------------ Element access -----------------------------
extern bool             hset_set(hset *se, hset_value val);
//extern bool             hset_lset(hset *se, long val);

extern bool             hset_del(hset *se, hset_value val);
// extern bool             hset_ldel(hset *se, long val);

extern bool             hset_get(const hset *se, hset_value val);
//extern bool             hset_lget(const hset *se, long val);

extern int              hset_cnt(const hset *se);   //TODO:
// ------------------------ PRINTERS/CHECKERS --------------------------

extern int              hset_techfprint(FILE *restrict out, const hset *se, int cnt);
static inline int       hset_techprint(const hset *se, int cnt);

extern bool             hset_validate(FILE *out, const hset *restrict se);

#endif /* !_HASHSET_H */
