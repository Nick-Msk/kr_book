#ifndef _VALUE64_H
#define _VALUE64_H

// ---------------------------------------------------------------------------------
// --------------------------- Public Faststring API -------------------------------
// ---------------------------------------------------------------------------------

// ----------------------------- Includes ------------------------------------------

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <float.h>
//
#include "bool.h"
#include "log.h"
#include "common.h"
#include "error.h"
#include "checker.h"
#include "fs.h"
#include "fs_iter.h"
#include "numeric_ops.h"
#include "getword.h"
#include "fileutils.h"

// --------------------------------- CONSTANTS AND GLOBALS --------------------------

// ---------------------------------- TYPES -----------------------------------------

// ANY type must be inside uint64_t
typedef union value64 {
        int                 ival;
        long                lval;
        double              dval;
        char               *sval;
        fs                 *fsval;
        void               *pval;
        uint64_t            u64;    // for hash
} value64;

_Static_assert(sizeof(value64) == sizeof(uint64_t),
               "value64 must be exactly as uint64_t");

typedef enum value64_type {
    VALUE64_UNKNOWN = 0,
    VALUE64_INT = 1,
    VALUE64_LNG,
    VALUE64_DBL,
    VALUE64_FS,
    VALUE64_PTR,
    VALUE64_STR,
    VALUE64_TYPE_COUNT
} value64_type;

typedef enum value64_serialize_type {
    VALUE64_2STR,
    VALUE64_2JSON       // not implemented
} value64_serialize_type;

typedef struct {
    const char  *name;
    size_t       size;
    bool         is_valid;
    const char  *type_desc;
} value64_typeinfo;

extern const                        value64_typeinfo* value64_info_get(value64_type typ);

static inline bool                  value64_checktype(value64_type typ) {
    return value64_info_get(typ) != NULL;
}

static inline const char            *value64_typename(value64_type t) {
    const value64_typeinfo* info = value64_info_get(t);
    if (!info)
        return userraise(NULL, ERR_UNSUPPORTED_TYPE, "Type %d not supported", t);
    return info->name;
}
extern  value64_type                 value64_gettype(const char *str);
// only zero for now
#define                 VALUE64_ZERO      (value64) {.u64 = 0L }

// getter for converters
typedef value64                     (*value64_ConverterFunc)(value64 v);
typedef value64                     (*value64_ConverterMoveFunc)(value64 *v);
// getter for comparators
typedef int                         (*value64_Comparator)(value64, value64);
// as void *, TODO: think if possible to be value *
typedef int                         (*value64_PComparator)(const void *restrict, const void *restrict);

/*#define                 VALUE64_INT(val)  (value64) {.u64 = 0L, .ival = val }
#define                 VALUE64_LONG(val) (value64) {.u64 = 0L, .lval = val }
#define                 VALUE64_DBL(val)  (value64) {.u64 = 0L, .dval = val }
#define                 VALUE64_PTR(val)  (value64) {.u64 = 0L, .pval = val }
// pointer copy!!!
#define                 VALUE64_STR(val)  (value64) {.u64 = 0L, .sval = val }
// local version
#define                 VALUE64_FSVALUE(val) (value64) {.u64 = 0L, .fsval = &(val) }
// pointer version TODO: not sure, commented for now

#define                 VALUE64_FSPVALUE(pval) (hset_value) {.fsval = fs_heapcreate(pval) }
//move version
#define                 VALUE64_FSMOVE(val)    (hset_value) {.fsval = fs_moveto_heap(val) } */




// ------------------------- CONSTRUCTOTS/DESTRUCTORS -------------------------------

// the part of mass creation API
// create value from pointer, value64 constructor ANY type, MOVE semantic
extern value64                      value64_pcopy_move(void *p, value64_type typ, bool move);
// copy, for array operations
static inline value64               value64_pinit(const void *p, value64_type typ){
    return value64_pcopy_move( (void *) p, typ, false);
}
// create value from pointer, value64 constructor ANY type, MOVE semantic
static inline value64               value64_pmove(void *p, value64_type typ){
    return value64_pcopy_move(p, typ, true);
}
// copy logic
static inline value64               value64_createint(int val){
    value64 tmp = VALUE64_ZERO;
    tmp.ival = val;
    return tmp;
}
static inline value64               value64_createlong(long lval){
    value64 tmp = VALUE64_ZERO;
    tmp.lval = lval;
    return tmp;
}
static inline value64               value64_createdbl(double dval){
    value64 tmp = VALUE64_ZERO;
    tmp.dval = dval;
    return tmp;
}
static inline value64               value64_createptr(void *pval){
    value64 tmp = VALUE64_ZERO;
    tmp.pval = pval;
    return tmp;
}
// TODO: movestr?
static inline value64               value64_createstr(const char *sval){
    if (!sval)
        userraiseint(ERR_NULLABLE_PTR, "Null pointer");
    value64 tmp = VALUE64_ZERO;
    if ( (tmp.sval = strdup(sval) ) == NULL)
        userraiseint(ERR_UNABLE_ALLOCATE, "Unable to dup c-string (%.20s)", sval);
    return tmp;
}
static inline value64               value64_createfs(const fs *fsval){
    if (!fsval || !fsval->v)
        userraiseint(ERR_NULLABLE_PTR, "Null pointer fs %p or fs->v %p", fsval, fsval ? fsval->v: NULL);
    value64 tmp = VALUE64_ZERO;
    if ( (tmp.fsval = fs_heapcreate(fsval) ) == NULL)
        userraiseint(ERR_UNABLE_ALLOCATE, "Unable to dup fs");
    return tmp;
}
// just switch over the types!
static inline value64               value64_clone(value64 source, value64_type typ){
    switch (typ){
        case VALUE64_INT:
            return value64_createint(source.ival);
        case VALUE64_LNG:
            return value64_createlong(source.lval);
        case VALUE64_DBL:
            return value64_createdbl(source.dval);
        case VALUE64_FS:
            return value64_createfs(source.fsval);
        case VALUE64_PTR:
            return value64_createptr(source.pval);
        case VALUE64_STR:
            return value64_createstr(source.sval);
        default:
            return VALUE64_ZERO;
    }
}
// move constructor
static inline value64               value64_movefs(const fs *fsval){
    value64 tmp = VALUE64_ZERO;
    if ( (tmp.fsval = fs_moveto_heap( (fs *) fsval) ) == NULL)
        userraiseint(ERR_UNABLE_ALLOCATE, "Unable to dup fs");
    return tmp;
}
static inline void                  value64_free(value64 v, value64_type typ){
    switch (typ){
        case VALUE64_STR:
            free(v.sval);   // even if NULL
        break;
        case VALUE64_FS:
            fs_free(v.fsval);   // even if NULL
        break;
        default:
        break;
    }
}
// destructor
static inline void                  value64_freefs(value64 v){
    return value64_free(v, VALUE64_FS);
}
static inline void                  value64_freestr(value64 v){
    return value64_free(v, VALUE64_STR);
}

// -------------------- ACCESS AND MODIFICATORS -------------------------------------
// just get
static inline int                   value64_int(value64 v){
    return v.ival;
}
static inline long                  value64_long(value64 v){
    return v.lval;
}
static inline double                value64_dbl(value64 v){
    return v.dval;
}
static inline char                 *value64_str(value64 v){
    return v.sval;
}
static inline void                 *value64_ptr(value64 v){
    return v.pval;
}
static inline fs                   *value64_fs(value64 v){
    return v.fsval;
}

// move to EXISTING object, thart is NOT a constructor
// move switcher to EXISTING object
extern value64                     *value64_move(value64 *restrict target, value64 *restrict source, value64_type typ);
static inline value64              *value64_move_int(value64 *restrict target, value64 *restrict source){
    return value64_move(target, source, VALUE64_INT);
}
static inline value64              *value64_move_long(value64 *restrict target, value64 *restrict source){
    return value64_move(target, source, VALUE64_LNG);
}
static inline value64              *value64_move_dbl(value64 *restrict target, value64 *restrict source){
    return value64_move(target, source, VALUE64_DBL);
}
static inline value64              *value64_move_str(value64 *restrict target, value64 *restrict source){
    return value64_move(target, source, VALUE64_STR);
}
static inline value64              *value64_move_ptr(value64 *restrict target, value64 *restrict source){
    return value64_move(target, source, VALUE64_PTR);
}
static inline value64              *value64_move_fs(value64 *restrict target, value64 *restrict source){
    return value64_move(target, source, VALUE64_FS);
}

extern unsigned long               value64_lhash(value64 value, value64_type typ);

// exchanger
extern void                        value64_exch(value64 *v1, value64 *v2);
// -------------------------------- Sorting/searching ------------------------------------------------
extern void                        value64_sort(value64_type typ, value64 *arr, int sz);
extern void                        value64_revsort(value64_type typ, value64 *arr, int sz);

// Сортировка по возрастанию для конкретных типов
static inline void value64_sort_int(value64 *arr, int sz) {
    value64_sort(VALUE64_INT, arr, sz);
}
static inline void value64_sort_long(value64 *arr, int sz) {
    value64_sort(VALUE64_LNG, arr, sz);
}
static inline void value64_sort_dbl(value64 *arr, int sz) {
    value64_sort(VALUE64_DBL, arr, sz);
}
static inline void value64_sort_str(value64 *arr, int sz) {
    value64_sort(VALUE64_STR, arr, sz);
}
static inline void value64_sort_fs(value64 *arr, int sz) {
    value64_sort(VALUE64_FS, arr, sz);
}
static inline void value64_sort_ptr(value64 *arr, int sz) {
    value64_sort(VALUE64_PTR, arr, sz);
}
// Сортировка по убыванию для конкретных типов
static inline void value64_revsort_int(value64 *arr, int sz) {
    value64_revsort(VALUE64_INT, arr, sz);
}
static inline void value64_revsort_long(value64 *arr, int sz) {
    value64_revsort(VALUE64_LNG, arr, sz);
}
static inline void value64_revsort_dbl(value64 *arr, int sz) {
    value64_revsort(VALUE64_DBL, arr, sz);
}
static inline void value64_revsort_str(value64 *arr, int sz) {
    value64_revsort(VALUE64_STR, arr, sz);
}
static inline void value64_revsort_fs(value64 *arr, int sz) {
    value64_revsort(VALUE64_FS, arr, sz);
}
static inline void value64_revsort_ptr(value64 *arr, int sz) {
    value64_revsort(VALUE64_PTR, arr, sz);
}

extern int                         value64_search(value64 val, value64_type typ, const value64 *arr, int sz);
extern int                         value64_revsearch(value64 val, value64_type typ, const value64 *arr, int sz);
// must be sorted acs
extern int                         value64_binsearch(value64 val, value64_type typ, const value64 *arr, int sz);
// order desc
extern int                         value64_rev_binsearch(value64 val, value64_type typ, const value64 *arr, int sz);
// SQL in low level
static inline bool                 value64_notin(value64 val, value64_type typ, const value64 *arr, int sz){
    return value64_search(val, typ, arr, sz) == -1;
}
static inline bool                 value64_in(value64 val, value64_type typ, const value64 *arr, int sz){
    return value64_search(val, typ, arr, sz) >= 0;
}
// value comparator
extern int                         value64_compare(value64 v1, value64 v2, value64_type typ);
// value
extern int                         value64_int_comp(value64 v1, value64 v2);
extern int                         value64_long_comp(value64 v1, value64 v2);
extern int                         value64_dbl_comp(value64 v1, value64 v2);
extern int                         value64_fs_comp(value64 v1, value64 v2);
extern int                         value64_str_comp(value64 v1, value64 v2);
extern int                         value64_ptr_comp(value64 v1, value64 v2);
// reverse
extern int                         value64_int_rev_comp(value64 v1, value64 v2);
extern int                         value64_long_rev_comp(value64 v1, value64 v2);
extern int                         value64_dbl_rev_comp(value64 v1, value64 v2);
extern int                         value64_fs_rev_comp(value64 v1, value64 v2);
extern int                         value64_str_rev_comp(value64 v1, value64 v2);
extern int                         value64_ptr_rev_comp(value64 v1, value64 v2);
// pointer comparator
extern int                         value64_pt_compare(const value64 *restrict v1, const value64 *restrict v2, value64_type typ);
// pointer comparators, for qsort, bsearch etc...
extern int                         value64_pint_comp (const void *restrict v1, const void *restrict v2);
extern int                         value64_plong_comp(const void *restrict v1, const void *restrict v2);
extern int                         value64_pdbl_comp (const void *restrict v1, const void *restrict v2);
extern int                         value64_pptr_comp (const void *restrict v1, const void *restrict v2);
extern int                         value64_pstr_comp (const void *restrict v1, const void *restrict v2);
extern int                         value64_pfs_comp  (const void *restrict v1, const void *restrict v2);
// rev
extern int                         value64_pint_rev_comp (const void *restrict v1, const void *restrict v2);
extern int                         value64_plong_rev_comp(const void *restrict v1, const void *restrict v2);
extern int                         value64_pdbl_rev_comp (const void *restrict v1, const void *restrict v2);
extern int                         value64_pptr_rev_comp (const void *restrict v1, const void *restrict v2);
extern int                         value64_pstr_rev_comp (const void *restrict v1, const void *restrict v2);
extern int                         value64_pfs_rev_comp  (const void *restrict v1, const void *restrict v2);

// pointer version, now using switch, w/o table
static inline value64_PComparator  value64_getPComparator(value64_type typ){
    switch (typ){
        case VALUE64_INT:
            return value64_pint_comp;
        case VALUE64_LNG:
            return value64_plong_comp;
        case VALUE64_DBL:
            return value64_pdbl_comp;
        case VALUE64_FS:
            return value64_pfs_comp;
        case VALUE64_PTR:
            return value64_pptr_comp;
        case VALUE64_STR:
            return value64_pstr_comp;
        default:
            userraiseint(ERR_UNSUPPORTED_TYPE, "%s: %d", value64_typename(typ), typ);
            return NULL;
    }
}
// reverse, pointer, now using switch, w/o table
static inline value64_PComparator  value64_getPRevComparator(value64_type typ){
    switch (typ){
        case VALUE64_INT:
            return value64_pint_rev_comp;
        case VALUE64_LNG:
            return value64_plong_rev_comp;
        case VALUE64_DBL:
            return value64_pdbl_rev_comp;
        case VALUE64_FS:
            return value64_pfs_rev_comp;
        case VALUE64_PTR:
            return value64_pptr_rev_comp;
        case VALUE64_STR:
            return value64_pstr_rev_comp;
        default:
            userraiseint(ERR_UNSUPPORTED_TYPE, "%s: %d", value64_typename(typ), typ);
            return NULL;
    }
}
// value, now using switch, w/o table
static inline value64_Comparator    value64_getComparator(value64_type typ){
    switch (typ){
        case VALUE64_INT:
            return value64_int_comp;
        case VALUE64_LNG:
            return value64_long_comp;
        case VALUE64_DBL:
            return value64_dbl_comp;
        case VALUE64_FS:
            return value64_fs_comp;
        case VALUE64_PTR:
            return value64_ptr_comp;
        case VALUE64_STR:
            return value64_str_comp;
        default:
            userraiseint(ERR_UNSUPPORTED_TYPE, "%s: %d", value64_typename(typ), typ);
            return NULL;
    }
}
// value, now using switch, w/o table
static inline value64_Comparator  value64_getRevComparator(value64_type typ){
    switch (typ){
        case VALUE64_INT:
            return value64_int_rev_comp;
        case VALUE64_LNG:
            return value64_long_rev_comp;
        case VALUE64_DBL:
            return value64_dbl_rev_comp;
        case VALUE64_FS:
            return value64_fs_rev_comp;
        case VALUE64_PTR:
            return value64_ptr_rev_comp;
        case VALUE64_STR:
            return value64_str_rev_comp;
        default:
            userraiseint(ERR_UNSUPPORTED_TYPE, "%s: %d", value64_typename(typ), typ);
            return NULL;
    }
}
// ----------------------------- CONVERTERS ----------------------------------------

extern value64                     value64_convert(value64 v, value64_type from, value64_type to);
extern bool                        value64_is_convertable(value64 v, value64_type from, value64_type to);

// --- Группа INT ---
extern value64                     value64_convert_int_to_lng(value64 v);
extern value64                     value64_convert_int_to_dbl(value64 v);
extern value64                     value64_convert_int_to_fs(value64 v);
extern value64                     value64_convert_int_to_str(value64 v);
// --- Группа LNG ---
extern value64                     value64_convert_lng_to_int(value64 v);
extern value64                     value64_convert_lng_to_dbl(value64 v);
extern value64                     value64_convert_lng_to_fs(value64 v);
extern value64                     value64_convert_lng_to_str(value64 v);
// --- Группа DBL ---
extern value64                     value64_convert_dbl_to_int(value64 v);
extern value64                     value64_convert_dbl_to_lng(value64 v);
extern value64                     value64_convert_dbl_to_fs(value64 v);
extern value64                     value64_convert_dbl_to_str(value64 v);
// --- Группа FS ---
extern value64                     value64_convert_fs_to_fs(value64 v);
extern value64                     value64_convert_fs_to_int(value64 v);
extern value64                     value64_convert_fs_to_lng(value64 v);
extern value64                     value64_convert_fs_to_dbl(value64 v);
extern value64                     value64_convert_fs_to_str(value64 v);
// --- Группа STR ---
extern value64                     value64_convert_str_to_str(value64 v);
extern value64                     value64_convert_str_to_int(value64 v);
extern value64                     value64_convert_str_to_lng(value64 v);
extern value64                     value64_convert_str_to_dbl(value64 v);
extern value64                     value64_convert_str_to_fs(value64 v);

// MOVE semantic
extern value64                     value64_convert_move(value64 *source, value64_type from, value64_type to);

extern value64                     value64_convert_move_fs_to_str(value64 *v);
extern value64                     value64_convert_move_fs_to_fs(value64 *v);
extern value64                     value64_convert_move_str_to_fs(value64 *v);
extern value64                     value64_convert_move_str_to_str(value64 *v);
// ------------------------ PRINTERS/CHECKERS ---------------------------------------

// generic file serilization!!
extern int                          value64_fprint(FILE *restrict out, const char *restrict msg, value64 val, value64_type typ);
static inline void                  value64_log(value64 val, value64_type typ){
    value64_fprint(logfile, 0, val, typ);
}
static inline void                  value64_print(value64 val, value64_type typ){
    value64_fprint(stdout, 0, val, typ);
}
// typed
extern int                          value64_fprint_str(FILE *restrict out, value64 val);
extern int                          value64_fprint_int(FILE *restrict out, value64 val);
extern int                          value64_fprint_lng(FILE *restrict out, value64 val);
extern int                          value64_fprint_dbl(FILE *restrict out, value64 val);
extern int                          value64_fprint_fs(FILE *restrict out, value64 val);
extern int                          value64_fprint_ptr(FILE *restrict out, value64 val);

// --------------------------------- SERIALIZATION ----------------------------------

// file readers
// f must be open for read, fs must be initialized, val can be NULL, it means just check
/* NOT IMPLEMENTE YET, value64_freadval call value64_sreadval_<type>
extern bool                         value64_readval_str(FILE *restrict f, value64 *restrict val, fs *restrict buf);
extern bool                         value64_readval_int(FILE *restrict f, value64 *restrict val, fs *restrict buf);
extern bool                         value64_readval_lng(FILE *restrict f, value64 *restrict val, fs *restrict buf);
extern bool                         value64_readval_dbl(FILE *restrict f, value64 *restrict val, fs *restrict buf);
// extern bool                         value64_readval_ptr(FILE *restrict f, value64 *restrict val, fs *restrict buf); // not supported!

extern bool                         value64_sreadval_fs(value64 *restrict val, fs *restrict buf);
*/
// string readers!
// fs must be initialized, val can be NULL, it means just check
extern bool                         value64_sreadval_str(value64 *restrict val, fs *restrict buf);
extern bool                         value64_sreadval_int(value64 *restrict val, fs *restrict buf);
extern bool                         value64_sreadval_lng(value64 *restrict val, fs *restrict buf);
extern bool                         value64_sreadval_dbl(value64 *restrict val, fs *restrict buf);

// generic reader, NOTE: it calls value64_sreadval_<type>
extern bool                         value64_freadval(FILE *restrict out, value64_type typ, value64 *restrict val, fs *restrict buf);
// generic save/load
extern int                          value64_fsave(FILE *out, value64 val, value64_type typ, bool savetypeinfo);
extern bool                         value64_fload(FILE *restrict out, value64 *restrict val, value64_type typ, bool loadtypeinfo, fs *restrict buf);

// generic to string TODO: fs MUST be initialized
extern int                          value64_tostr(fs *target, value64 val, value64_type typ, value64_serialize_type serit);
// type to string TODO:
extern int                          value64_tostr_str(fs *target, value64 val);
extern int                          value64_tostr_int(fs *target, value64 val);
extern int                          value64_tostr_lng(fs *target, value64 val);
extern int                          value64_tostr_dbl(fs *target, value64 val);
extern int                          value64_tostr_fs(fs *target, value64 val);

// ------------------------------------ ETC. ----------------------------------------

#endif /* !_VALUE64_H */

