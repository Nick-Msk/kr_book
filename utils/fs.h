#ifndef _FS_H
#define _FS_H

// ---------------------------------------------------------------------------------
// --------------------------- Public Faststring API -------------------------------
// ---------------------------------------------------------------------------------

// --------------- Includes -----------------------

#include <stdio.h>
#include <string.h>
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
             , FS_FLAG_ALLOC  = 0x0     // standard allocation
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
#if defined(FS_ALLOCATOR)
    int         pos;    //
#endif
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
    return fl == FS_FLAG_ALLOC; // heap marker
}

static inline bool          fs_alloc(const fs*s){
    return fs_flag_alloc(s->flags);
}

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

#define FSEMPTY (fs){.sz = 0, .len = 0, .flags = FS_FLAG_STATIC, .v = ""};

#define FSINITSTATIC(...)  (fs){.sz = 1, .len = 0, .flags = FS_FLAG_STATIC, .v = "", ##__VA_ARGS__}

#define FS(...)   (fs){.sz = 0, .len = 0, .flags = FS_FLAG_ALLOC, .v = 0, ##__VA_ARGS__}

#define fsfree(s) fs_free(&(s))

// destructor, macro wrapper will be
static inline void          fs_free(fs *s){
    if (fs_alloc(s)){
        logsimpleact(free(s->v), "freed...");   // WOW, logsimpleact?
        s->sz = s->len = 0;
        s->v = 0;
    }
}

extern fs                   fsinit(int sz);

static inline fs            fsempty(void){
    return FS();
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

#if defined(FS_ALLOCATOR)
// detach from allocator! Must be freed manually
extern bool                 fsdetach(fs *s);
#endif

// free all allocated
extern void                 fsfreeall(void);

// TODO: fs_const()

// -------------------- ACCESS AND MODIFICATORS ------------------------

// direct access, NO change len or sz, position MUST be < sz
static inline char          *fs_get(const fs *s, int pos){
    //return logsimpleret(s->v + pos, "Getting %p[%c]", s->v + pos, s->v[pos]);
    return s->v + pos;
}

// automatically adjust len (??) and sz (realloc)
extern char                 *fs_elem(fs *s, int pos);

static inline char          *fs_setlen(fs *s, int poslen){
    s->len = poslen;
    *fs_elem(s, poslen) = '\0';
    return fs_get(s, poslen);
}

static inline int           fslen(fs s){
    return s.len;
}

static inline int           fssz(fs s){
    return s.sz;
}

// shrink to real len + 1 ( + 1 because '\0' is ASSUMED)
extern fs                   *fs_shrink(fs *s);

extern fs                   *fs_resize(fs *s, int newsz);

static inline fs            *fs_increase(fs *s, int inc){
    return fs_resize(s, s->sz + inc);
}

static inline char          *fs_str(fs *s){
    return s->v;
}

static inline const char    *fs_strcopy(fs *s){
    return (const char *) strdup(s->v);
}

static inline int            fscmp(fs str1, fs str2){
    return strcmp(str1.v, str2.v);
}

static inline int            fsicmp(fs str1, fs str2){
    return strcasecmp(str1.v, str2.v);
}

static inline void           fs_exch(fs *s1, fs *s2){
    fs tmp = *s1;
    *s1 = *s2;
    *s2 = tmp;
}

static inline fs             fs_reverse(fs str){
    reverse(str.v, str.len);
    return str;
}

#define                      get(s, pos) *fs_get(&(s), (pos) )
#define                      getv(s) (s.v)
#define                      elem(s, pos) *fs_elem( &(s), (pos) )

#define                      fsetlen(s, poslen) *fs_setlen( (&s), (poslen) )
#define                      fsend(s, poslen) *fs_setlen( (&s), (poslen) )

#define                      fsstr(s) fs_str(&(s) )

#define                      fsincrease(s, inc) fs_increase( &(s), (inc) )

#define                      fsshrink(s) fs_shrink(&(s) )

// ------------------------ PRINTERS/CHECKERS --------------------------

extern int                   fs_techfprint(FILE *restrict out, const fs *restrict s);
static inline int            fs_techprint(const fs *s){
    return fs_techfprint(stdout, s);
}

extern bool                  fs_validate(FILE *restrict out, const fs *restrict s);

// --------------------------- ETC. -------------------------

extern int                   FS_MIN_ACCOC;
extern int                   FS_TECH_PRINT_COUNT;

#endif /* !_FS_H */

