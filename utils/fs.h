#ifndef _FS_H
#define _FS_H

// ---------------------------------------------------------------------------------
// --------------------------- Public Faststring API -------------------------------
// ---------------------------------------------------------------------------------

// --------------- Includes -----------------------

#include "bool.h"
#include "log.h"
#include "common.h"
#include "error.h"

// ----------- CONSTANTS AND GLOBALS ---------------

// ------------------- TYPES -----------------------

typedef enum  {FS_FLAG_STATIC = 0x1
             , FS_FLAG_CONST  = 0x2     // Not user for now
             , FS_FLAG_LOCAL  = 0x4     // not used in this version
             , FS_FLAG_ALLOC  = 0x8     // standard allocation
} FS_FLAGS;

// type-support functions

// to string
static inline const char * fs_flag_str(FS_FLAGS flag){
    switch(flag){
        CASE_RETURN(FS_FLAG_STATIC);        // not sure whether is good to use CASE_RETURN
        CASE_RETURN(FS_FLAG_CONST);
        CASE_RETURN(FS_FLAG_LOCAL);
        CASE_RETURN(FS_FLAG_ALLOC);
        default:         return "Unknown action";
    }
}

// faststring
typedef struct fs {
    int         len, sz; // sz >= len + 1 because of last '\0'
    FS_FLAGS    flags;
    char       *v;      // with '\0'
} fs;

// flag checkers
static inline bool fs_flag_static(FS_FLAGS fl){
    return fl & FS_FLAG_STATIC;
}

static inline bool fs_statis(const fs *s){
    return fs_flag_static(s->flags);
}

static inline bool fs_flag_const(FS_FLAGS fl){
    return fl & FS_FLAG_CONST;
}

static inline bool fs_const(const fs *s){
    return fs_flag_const(s->flags);
}

static inline bool fs_flag_local(FS_FLAGS fl){
    return fl & FS_FLAG_LOCAL;
}

static inline bool fs_local(const fs *s){
    return fs_flag_local(s->flags);
}

static inline bool fs_flag_alloc(FS_FLAGS fl){
    return fl & FS_FLAG_ALLOC; // heap marker
}

static inline bool fs_heap(const fs*s){
    return fs_flag_heap(s->flags);
}

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

// destructor, macro wrapper will be
static inline void          fs_free(fs *s){
    if (fs_alloc(s))
        logsimpleact(free(s->s), "freed...");   // WOW, logsimpleact?
    s->sz = s->len = s->flags = 0;
    s->v = 0;
}

// direct access, NO change len or sz, position MUST be < sz
static inline char          *fs_get(const fs *s, int pos){
    return logsimpleret(s->v + pos, "Getting %p[%c]", s->s + pos, s->s[pos]);
}

// automatically adjust len (??) and sz (realloc)
extern char                 *fs_elem(fs *s, int pos);

// shrink to real len + 1 ( + 1 because '\0' is ASSUMED)
extern fs                   *fs_shrink(fs *s);

// ----------------- PRINTERS/CHECKERS ----------------------

int                     fs_techfprint(FILE *restrict out, const fs *restrict s);
static inline int       fs_techprint(const fs *restrict s){
    return fs_techfprint(stdout, s);
}

// --------------------------- ETC. -------------------------

// common initializer
#define FSEMPTY (fs){.sz = 1, .len = 0, .flags = FS_FLAG_STATIC, .sz = ""};

#endif /* !_FS_H */

