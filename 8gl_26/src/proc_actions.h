#ifndef _PROC_ACTIONS_H
#define _PROC_ACTIONS_H

#include <stdio.h>
#include "bool.h"
#include "getword.h"
#include "files.p8.5.h"


typedef struct Context Context;
typedef struct Command Command;     // to be able to add to Context
// THAT WILL BE MOVED OUT!!! TODO:
typedef int         (*process_unit)(Context *c);

typedef struct Command {
    const char *    name;
    const char *    desc;
    process_unit    proc;
} Command;
// --------------------------- OUT TILL HERE -----------------

typedef struct Context {
    Lexem       *lex;
    MFILE       *mfr;   // read
    MFILE       *mfw;   // write
    bool        quit;   // flag for quit
    Command    *cmds;   // link to all commands
    // ....
} Context;
#define             ContextInit(...) (Context) {.lex = 0, .mfr = 0, .mfw = 0, .quit = false,  ##__VA_ARGS__}

// ------------------------------------------ Utilities -------------------------------------------

extern int                 Context_techfprint(FILE *restrict out, const Context *restrict ctx);

static inline int          Context_techprint(const Context *ctx){
    return Context_techfprint(stdout, ctx);
}

// --------------------------------------- AUXILLARY PROCS -------------------------------------------

extern int                 proc_quit(Context *ctx);
extern int                 proc_help(Context *ctx);
extern int                 proc_techprint(Context *ctx);

// ------------------------------------------- MFILE PROCS --------------------------------------------
extern int                 proc_create(Context *ctx);
extern int                 proc_create(Context *ctx);

#endif /* !_PROC_ACTIONS_H */
