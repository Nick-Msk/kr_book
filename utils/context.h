#ifndef _CONTEXT_H
#define _CONTEXT_H

// ---------------------------------------------------------------------------------
// --------------------------- Public Context API -------------------------------
// ---------------------------------------------------------------------------------

// --------------------- Includes (no FS here to avoid recursive deps)  -----------------------

#include <stdio.h>
#include <string.h>
//
#include "bool.h"
#include "log.h"
#include "common.h"
#include "error.h"
#include "checker.h"

// ----------- CONSTANTS AND GLOBALS ---------------

enum { ContextElemActive = 0x1 };

// ------------------- TYPES -----------------------

// very simple, no groups or smth like that
typedef struct ContextElem {
    char        *name;
    char        *value; // LIST of value must be here!!! TODO:
    unsigned     flags;
} ContextElem;

typedef struct Context {
    int             cnt;
    ContextElem    *ctx;
} Context;

// ------------------ CONSTRUCTOTS/DESTRUCTORS -----------------------

extern Context              *ctxinit(int sz);
extern void                  ctxfreed(Context *ctx);

#define                     ctxfree(c) (ctxfreed(c), c = 0)

// -------------------- ACCESS AND MODIFICATORS ------------------------

extern const char           *ctxget(const Context *restrict ctx, const char *restrict name);
static inline bool           ctxexists(const Context *restrict ctx, const char *restrict name){
    return ctxget(ctx, name) != 0;
}
extern bool                  ctxadd(Context *restrict ctx, const char *restrict name, const char *restrict value);
extern bool                  ctxdel(Context *restrict ctx, const char *restrict name);
extern bool                  ctxreset(Context *ctx);

// ------------------------ PRINTERS/CHECKERS --------------------------

extern int                   ctxfprint(FILE *restrict out, const Context *restrict ctx);
static inline int            ctxprint(const Context *ctx){
    return ctxfprint(stdout, ctx);
}

#endif /* !_CONTEXT_H */

