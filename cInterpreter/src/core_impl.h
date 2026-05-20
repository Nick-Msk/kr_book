#ifndef _CORE_IMPL_H
#define _CORE_IMPL_H

#include <stdio.h>
#include "bool.h"
#include "command_executor.h"
#include "fs.h"
#include "fs_array.h"
#include "getword.h"

// renamed from Runtimedata
typedef struct Runtimedata {
    Lexem       *lex;
    bool        quit;   // flag for quit
    Command    *cmds;   // link to all commands
    // ----- TODO: use a separate structure for real data
    // real data from here
    fsarray    include;        // not user for now
    fsarray    functions;      // not user for now
    fsarray    body;           // code
} Runtimedata;
#define             RuntimedataInit(...) (Runtimedata) {.include = fsarr_empty(), .functions = fsarr_empty(), .body = fsarr_init(100), .quit = false,  ##__VA_ARGS__}

// ------------------------------------------ Utilities -------------------------------------------

extern int                 Runtimedata_techfprint(FILE *restrict out, const Runtimedata *restrict rt);

static inline int          Runtimedata_techprint(const Runtimedata *rt){
    return Runtimedata_techfprint(stdout, rt);
}

// --------------------------------------- AUXILLARY PROCS -------------------------------------------

extern int                 proc_quit(Runtimedata *rt);
extern int                 proc_help(Runtimedata *rt);
extern int                 proc_techprint(Runtimedata *rt);

// ------------------------------------------- MFILE PROCS --------------------------------------------

#endif /* !_CORE_IMPL_H */
