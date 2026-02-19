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
    return i->pos > i->to;
}

static inline char         *fsnextrev(fsiterrev *i){
    return i->v + i->pos--;
}

static inline char         *fscurrrev(fsiterrev *i){
    return i->v + i->pos;
}

#define                     hasnext(i) fshasnext(&(i) )
#define                     next(i) *(fsnext(&(i) ) )
#define                     curr(i) *(fscurr(&(i) ) )
#define                     fsforeach(s, i) for (fsiter i = fseach(s); hasnext(i); next(i) )

#define                     hasnextrev(i) fshasnextrev(&(i) )
#define                     nextrev(i) *(fsnextrev(&(i) ) )
#define                     currrev(i) *(fscurrrev(&(i) ) )
#define                     fsforeachrev(s, i) for (fsiterrev i = fseachrev(s); hasnextrev(i); nextrev(i) )

// in future
// #define lyamdba(s, compare) ...

//
// foreach(s, i)
//     get(s, i) = 'u';
// for (fsiter i = fsaech(s)/fsslice(s, 2, 11); hasnext(i); next(i))
        // get(i) = 'y';    // or s.v[i.pos] = 't';
// for (int i = 0; i < s.len; i++)
//  s.v[i] = ...;

// ------------------------ PRINTERS/CHECKERS --------------------------

// --------------------------- ETC. -------------------------

#endif /* !_FS_ITER_H */

