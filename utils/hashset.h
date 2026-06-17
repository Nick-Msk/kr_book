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
    { HSET_INT = 1, HSET_LONG, HSET_DBL, HSET_FS, HSET_PTR,
      HSET_HEAP_ALLOC = 0x101,      // hset is allocated by malloc
      HSET_UKNOWN = -1 }
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
            default:
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

// --------------------------------------- hset ----------------------------------------------
#define                 HSET(size, typ) (hset) {.sz = (size), .flags = (typ), .table = 0 }
#define                 HSET_NONINIT        HSET(0, HSET_UKNOWN)
// ---------------------------- hset_value: TODO: refactor to separate value.c (Value64 type)
#define                 HSET_ZERO_VALUE     (hset_value) {.u64 = 0L }
#define                 HSET_INTVALUE(val)  (hset_value) {.u64 = 0L, .ival = val }
#define                 HSET_LONGVALUE(val) (hset_value) {.u64 = 0L, .lval = val }
#define                 HSET_DBLVALUE(val)  (hset_value) {.u64 = 0L, .dval = val }
#define                 HSET_PTRVALUE(val)  (hset_value) {.u64 = 0L, .pval = val }
// local version
#define                 HSET_FSVALUE(val)   (hset_value) {.fsval = &(val) }
// pointer version
#define                 HSET_FSPVALUE(pval) (hset_value) {.fsval = hset_create_fs(pval) }
//move version
#define                 HSET_FSMOVE(val)    (hset_value) {.fsval = fs_moveall(val) }


// create value from pointer
extern hset_value           hset_createval(const void *p, hset_type typ);

//  check if in non-init state
static inline bool          hset_isnoninit(const hset *se){
    return se->flags & HSET_UKNOWN && se->sz == 0;
}

static inline hset_type     hset_getype(const hset *se){
    return se->flags & 0xFF;
}

static inline bool          hset_heap_alloc(const hset *se){
    return se->flags & HSET_HEAP_ALLOC;
}

static inline void          hset_set_heap_alloc(hset *se){
    se->flags |= HSET_HEAP_ALLOC;
}

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

extern hset                 hset_init(int sz, hset_type typ);      // #define will be for particular type
extern hset                 hset_init_resize(hset *se, int newsz);
extern hset                 hset_normalize(hset *se);
//
extern void                 hset_free(hset *se);
//
extern hset                 hset_clone(const hset *se);

extern hset                 hset_cloneas(const hset *se, hset_type typ);

// universale loader
extern hset                 hset_fromanyarr(const void *arr, int sz, hset_type typ);
// typed
static inline hset          hset_fromiarr(const int *iarr, int sz){
    return hset_fromanyarr(iarr, sz, HSET_INT);
}
// not SURE TODO:
static inline hset          hset_fromfsarr(/*fs_array iarr*/ const fs *fsarr, int sz){
    return hset_fromanyarr(fsarr, sz, HSET_FS);
}
static inline hset          hset_fromlarr(const long *larr, int sz){
    return hset_fromanyarr(larr, sz, HSET_LONG);
}
static inline hset          hset_fromdarr(const double *darr, int sz){
    return hset_fromanyarr(darr, sz, HSET_DBL);
}
extern hset                 hset_fromparr(const void **parr, int sz){
    return hset_fromanyarr(parr, sz, HSET_PTR);
}
// just intersect with construct
extern hset                 hset_init_intersect(const hset *restrict se1, const hset *restrict se2);
// minus with construct with construct
extern hset                 hset_init_minus(const hset *restrict se1, const hset *restrict se2);
// simm diff with construct
extern hset                 hset_init_symmdiff(const hset *restrict a, const hset *restrict b);
// union with construct
extern hset                 hset_init_union(const hset *restrict a, const hset *restrict b);

// -------------------- ACCESS AND MODIFICATORS ------------------------

// ------------------------ Element access -----------------------------
// true if new element is added, if exists - false
extern bool                 hset_set(hset *se, hset_value val);
// try to delete elemenet, true if deleted, false if not found
extern bool                 hset_del(hset *se, hset_value val);

extern bool                 hset_get(const hset *se, hset_value val);

static inline int           hset_cnt(const hset *se){
    return se->count;
}
static inline bool          hset_isempty(const hset *se) {
    return hset_cnt(se) == 0;
}

extern void                 hset_clean(hset *se);
// origin wll be cleaned
static inline hset         *hset_move(hset *target, hset * origin){
    hset_free(target);
    *target = *origin;
    origin->table = 0;
    origin->flags = origin->sz = 0;
    return logsimpleret(target, "moved to %p, sz %d, cnt %d", target, target->sz, target->count);
}

extern bool                 hset_elem_move(hset *restrict se, hset_elem *restrict elem);

extern bool                 hset_eq(const hset *restrict se1, const hset *restrict se2);

extern bool                 hset_noteq(const hset *restrict se1, const hset *restrict se2);
// load values from array, any type
extern int                  hset_loadanyarr(hset *restrict se, const void *arr, int sz, hset_type typ);

static inline int           hset_loadiarr(hset *restrict se, const int *iarr, int sz){
    return hset_loadanyarr(se, iarr, sz, HSET_INT);
}
static inline int           hset_loadlarr(hset *restrict se, const long *larr, int sz){
    return hset_loadanyarr(se, larr, sz, HSET_LONG);
}
static inline int           hset_loaddarr(hset *restrict se, const double *darr, int sz){
    return hset_loadanyarr(se, darr, sz, HSET_DBL);
}
static inline int           hset_loadparr(hset *restrict se, const void * const *restrict parr, int sz){
    return hset_loadanyarr(se, parr, sz, HSET_PTR);
}
// TODO: ???
static inline int           hset_loadfsarr(hset *restrict se, const fs *restrict fsarr, int sz){
    return hset_loadanyarr(se, fsarr, sz, HSET_FS);
}
// check if all of se2 in se1 strictly or not
extern bool                 hset_subset_check(const hset *restrict se1, const hset *restrict se2, bool strict);
// check if all of se2 in se1
static inline bool          hset_in(const hset *restrict se1, const hset *restrict se2){
    return hset_subset_check(se1, se2, false);
}
// check if all of se2 in se1  but se2 not equal se1
static inline bool          hset_strictin(const hset *restrict se1, const hset *restrict se2){
    return hset_subset_check(se1, se2, true);
}
// if not exists
extern bool                 hset_notexists(const hset *restrict se1, const hset *restrict se2);
// if exists any of se2 in se1
extern bool                 hset_any(const hset *restrict se1, const hset *restrict se2);
// se1 -= se2 as SET
extern hset                *hset_minus(hset *restrict se1, const hset *restrict se2);
// se1 insersect= se2 as SET
extern hset                *hset_intersect(hset *restrict se1, const hset *restrict se2);
// se1 symmdiff= se2 as SET
extern hset                *hset_symmdiff(hset *restrict a, const hset *restrict b);
// union= as SET
extern hset                *hset_union(hset *restrict a, const hset *restrict b);

// ------------------------------------- PRINTERS/CHECKERS ---------------------------------

extern int                  hset_techfprint(FILE *restrict out, const hset *se, int cnt);
static inline int           hset_techprint(const hset *se, int cnt){
    return hset_techfprint(stdout, se, cnt);
}

extern bool                 hset_validate(FILE *out, const hset *restrict se);
// --------------------------------- SERIALIZATION -----------------------------------------
extern int                  hset_fsave(FILE  *restrict out, const hset *se);
extern int                  hset_save(const char *restrict fname, const hset *se);
extern int                  hset_fload(FILE *restrict in, hset *restrict se);
extern int                  hset_load(const char *restrict fname, hset *restrict se);


// const
typedef                     void (*hset_const_proc_t)(hset_value v);
// change //typedef                 void (*hset_proc_t)(hset_value *v);
// modift structure typedef                 void (*hset_modify_proc_t)(hset *se, hset_elem *el);

typedef                     void * pointer_to_void;

extern void                 hset_const_foreach(const hset *se, hset_const_proc_t proc);
//extern void               hset_foreach(hset *se, hset_proc_t proc);
//extern void               hset_modify_foreach(hset *se, hset_modify_proc_t proc);

// Общий внутренний макрос – проходит по всем элементам и на каждой итерации
// объявляет var заданного типа и присваивает ей значение из поля field.
#define _HSET_FOREACH_TYPE(se, var, type, field) \
    for (int _i_ = 0; _i_ < (se)->sz; _i_++) \
        for (const hset_elem *_el_ = (se)->table[_i_]; _el_; _el_ = _el_->next) \
            for (int _flag_ = 1; _flag_; _flag_ = 0) \
                for (type var = _el_->v.field; _flag_; _flag_ = 0)

            //_Pragma("GCC diagnostic push") \
            //_Pragma("GCC diagnostic ignored \"-Wfor-loop-analysis\"") \
           // _Pragma("GCC diagnostic pop")
// Публичные макросы
#define                     HSET_FOREACH_INT(se, var)  _HSET_FOREACH_TYPE(se, var, int, ival)
#define                     HSET_FOREACH_LONG(se, var)  _HSET_FOREACH_TYPE(se, var, long, lval)
#define                     HSET_FOREACH_DBL(se, var)  _HSET_FOREACH_TYPE(se, var, double, dval)
#define                     HSET_FOREACH_PTR(se, var)  _HSET_FOREACH_TYPE(se, var, void*, pval)
// any type
#define                     HSET_FOREACH(se, var) \
        for (int _i_ = 0; _i_ < (se)->sz; _i_++) \
            for (const hset_elem *_el_ = (se)->table[_i_]; _el_; _el_ = _el_->next) \
                for (int _flag_ = 1; _flag_; _flag_ = 0) \
                    for (hset_value var  = _el_->v; _flag_; _flag_ = 0)

// ----------------------------------------- REDUCE -----------------------------------------
typedef struct              hset_accum {
    hset_value  value;    // накопленное значение (сумма, максимум и т.п.)
    int         count;    // количество элементов, участвовавших в накоплении
    fs          str_agg;  // для будущей агрегации строк (пока можно не добавлять)
} hset_accum;

#define                     HSET_ACCUM(...)  (hset_accum) { .value = HSET_ZERO_VALUE, .count = 0, .str_agg = FS(), __VA_ARGS__} 
#define                     HSET_ACCUM_DBL_ZERO  (hset_accum) { .value = HSET_DBLVALUE(0.0), .count = 0, .str_agg = FS() } 

typedef                     void (*hset_reduce_func)(hset_accum *acc, hset_value v);
extern hset_accum           hset_initreduce(const hset *se, hset_accum init, hset_reduce_func func);

static inline hset_accum    hset_reduce(const hset *se, hset_reduce_func func){
    return hset_initreduce(se, HSET_ACCUM(), func);
}

// ------------------------------------- REDUCE IMPL -----------------------------------------
extern void                 hset_sum_int    (hset_accum *acc, hset_value v);
extern void                 hset_count_int  (hset_accum *acc, hset_value v);
extern void                 hset_max_int    (hset_accum *acc, hset_value v);
extern void                 hset_min_int    (hset_accum *acc, hset_value v);
//extern void               hset_avg_int    (hset_accum *acc, hset_value v);

extern void                 hset_sum_dbl    (hset_accum *acc, hset_value v);
extern void                 hset_count_dbl  (hset_accum *acc, hset_value v);
extern void                 hset_max_dbl    (hset_accum *acc, hset_value v);
extern void                 hset_min_dbl    (hset_accum *acc, hset_value v);
//extern void                 hset_avg_dbl    (hset_accum *acc, hset_value v);

#endif /* !_HASHSET_H */

