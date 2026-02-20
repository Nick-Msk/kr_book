#ifndef _FS_ITER_H
#define _FS_ITER_H

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
#include "fs.h"

// ----------- CONSTANTS AND GLOBALS ---------------

// ------------------- TYPES -----------------------

typedef struct fsiter {
    int     pos, from, to;
    char   *v;
} fsiter;

typedef struct fsiterrev {
    int     pos, from, to;
    char   *v;
} fsiterrev;

typedef struct fsnew {
    int     pos;
    fs     *s;
} fsnew;

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

// TODO: make a REV interator as fsreviter{ };

// forward iterator
static inline fsiter        fseach(fs s){
    return (fsiter){.pos = 0, .from = 0, .to = s.len, .v = s.v};
}

// foprward iterator
static inline fsiterrev     fseachrev(fs s){
    return (fsiterrev){.pos = s.len - 1, .from = s.len - 1, .to = 0, .v = s.v};
}

// new iterator, it's assumed str is initialized as heap!
static inline fsnew         fsinew(fs *str){
    // fs_init
    return (fsnew){.pos = 0, .s = str};
}

// ++ slice, TODO: rev slice
static inline fsiter        fsslice(fs s, int from, int to){
    fsiter i = fseach(s);
    i.from = i.pos = from < s.len ? from : s.len;
    i.to = to < s.len ? to : s.len;
    return i; // (fsiter){.pos = MIN(from, s.len), .from = MIN(from, s.len), .to = MIN(to, s.len), .v = s.v};
}

// -------------------- ACCESS AND MODIFICATORS ------------------------

static inline bool          fshasnext(fsiter *i){
    return i->pos < i->to;
}

static inline char         *fsnext(fsiter *i){
    return i->v + i->pos++;
}

static inline char         *fscurr(fsiter *i){
    return i->v + i->pos;
}

static inline bool          fshasnextrev(fsiterrev *i){
    return i->pos >= i->to;
}

static inline char         *fsnextrev(fsiterrev *i){
    return i->v + i->pos--;
}

static inline char         *fscurrrev(fsiterrev *i){
    return i->v + i->pos;
}

// semi-constructor!
static inline char         *fselemnext(fsnew *i){
    return fs_elem(i->s, i->pos++);
}
// not sure if really need that
static inline char         *fselemcurr(fsnew *i){
    return fs_elem(i->s, i->pos);
}

// setup len and '\0'
static inline char         *fselemend(fsnew *i){
    return fs_setlen(i->s, i->pos);
}

// forward
#define                     hasnext(i)   fshasnext(&(i) )
#define                     next(i)     *(fsnext(&(i) ) )
#define                     curr(i)     *(fscurr(&(i) ) )
#define                     fsforeach(s, i) for (fsiter i = fseach(s); hasnext(i); next(i) )

// reverse
#define                     hasnextrev(i) fshasnextrev(&(i) )
#define                     nextrev(i)   *(fsnextrev(&(i) ) )
#define                     currrev(i)   *(fscurrrev(&(i) ) )
#define                     fsforeachrev(s, i) for (fsiterrev i = fseachrev(s); hasnextrev(i); nextrev(i) )

// fsnew
#define                     elemnext(i) *fselemnext(&(i) )
#define                     elemcurr(i) *fselemcurr(&(i) )
#define                     elemend(i)  *fselemend(&(i) )

// in future
// #define lyamdba(s, compare) ...

//
// foreach(s, i)
//     get(s, i) = 'u';
// for (... )
//    elemnext(is) = arr[i] + '0';
// for (fsiter i = fsaech(s)/fsslice(s, 2, 11); hasnext(i); next(i))
        // get(i) = 'y';    // or s.v[i.pos] = 't';
// for (int i = 0; i < s.len; i++)
//  s.v[i] = ...;

// ------------------------ PRINTERS/CHECKERS --------------------------

// --------------------------- ETC. -------------------------

#endif /* !_FS_ITER_H */

