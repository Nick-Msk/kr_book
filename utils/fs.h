#ifndef _FS_H
#define _FS_H

// ---------------------------------------------------------------------------------
// --------------------------- Public Faststring API -------------------------------
// ---------------------------------------------------------------------------------

// --------------- Includes -----------------------

#include <stdio.h>
#include <strings.h>
//
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
static inline bool          fs_flag_static(FS_FLAGS fl){
    return fl & FS_FLAG_STATIC;
}

static inline bool          fs_static(const fs *s){
    return fs_flag_static(s->flags);
}

static inline bool          fs_flag_const(FS_FLAGS fl){
    return fl & FS_FLAG_CONST;
}

static inline bool          fs_const(const fs *s){
    return fs_flag_const(s->flags);
}

static inline bool          fs_flag_local(FS_FLAGS fl){
    return fl & FS_FLAG_LOCAL;
}

static inline bool          fs_local(const fs *s){
    return fs_flag_local(s->flags);
}

static inline bool          fs_flag_alloc(FS_FLAGS fl){
    return fl & FS_FLAG_ALLOC; // heap marker
}

static inline bool          fs_alloc(const fs*s){
    return fs_flag_alloc(s->flags);
}

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

#define FSEMPTY (fs){.sz = 1, .len = 0, .flags = FS_FLAG_STATIC, .v = ""};

#define FSINITSTATIC(...)  (fs){.sz = 1, .len = 0, .flags = FS_FLAG_STATIC, .v = "", ##__VA_ARGS__}

#define FSINITALLOC(...)   (fs){.sz = 0, .len = 0, .flags = FS_FLAG_ALLOC, .v = 0, ##__VA_ARGS__}

#define fsfree(s) fs_free(&(s))

// destructor, macro wrapper will be
static inline void          fs_free(fs *s){
    if (fs_alloc(s))
        logsimpleact(free(s->v), "freed...");   // WOW, logsimpleact?
    s->sz = s->len = s->flags = 0;
    s->v = 0;
}

extern fs                   fsinit(int sz);

static inline fs            fsempty(void){
    return FSINITALLOC(.sz = 0, .v = 0, .flags = FS_FLAG_ALLOC);
}

static inline fs            fscopy(const char *str){
    int        sz  = strlen(str) + 1;
    fs         val = fsinit(sz);
    if (val.v){
        memcpy(val.v, str, sz);
        val.len = sz - 1;
    }
    return val;
}

extern fs                   fsclone(fs s);

static inline fs            fsliteral(const char *lit){
    fs s = FSEMPTY;
    s.v = (char *)lit;  // TODO: probably use a union
    s.len = strlen(lit);
    s.sz = s.len + 1;
    return s;
}

// TODO: fs_const()

// -------------------- ACCESS AND MODIFICATORS ------------------------

#define                     fsstr(s) fs_str(&(s))
#define                     fsshrink(s) fs_shrink(&(s))

// direct access, NO change len or sz, position MUST be < sz
static inline char          *fs_get(const fs *s, int pos){
    return logsimpleret(s->v + pos, "Getting %p[%c]", s->v + pos, s->v[pos]);
}

// automatically adjust len (??) and sz (realloc)
extern char                 *fs_elem(fs *s, int pos);

static inline char          *fs_len(fs *s, int poslen){
    *fs_elem(s, poslen) = '\0';
    s->len = poslen;
    return s->v + poslen;
}

// shrink to real len + 1 ( + 1 because '\0' is ASSUMED)
extern fs                   *fs_shrink(fs *s);

extern fs                   *fs_resize(fs *s, int newsz);

static inline const char    *fs_str(fs *s){
    return s->v;
}

static inline const char    *fs_strcopy(fs *s){
    return (const char *) strdup(s->v);
}

#define                      get(s, pos) *fs_get(&(s), (pos))
#define                      getv(s) (s.v)
#define                      elem(s, pos) *fs_elem( &(s), (pos))
#define                      fslen(s, poslen) *fs_len( (&s), (poslen))
// ------------------------ PRINTERS/CHECKERS --------------------------

int                          fs_techfprint(FILE *restrict out, const fs *restrict s);
static inline int            fs_techprint(const fs *s){
    return fs_techfprint(stdout, s);
}

// --------------------------- ETC. -------------------------

#endif /* !_FS_H */

