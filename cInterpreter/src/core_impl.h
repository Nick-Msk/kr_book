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
    Lexem            lex;
    bool             quit;   // flag for quit
    Command         *cmds;   // link to all commands
    // ----- TODO: use a separate structure for real data

    // real data from here
    fsarray         include;        // not user for now
    fsarray         functions;      // not user for now
    fsarray         body;           // code
    FILE           *fl, *runfl;
    const char     *flname, *runflname;
} Runtimedata;

#define             RuntimedataInit(...) (Runtimedata)\
{.include = fsarr_empty(), .functions = fsarr_empty(), .body = fsarr_empty(), .quit = false, .fl = 0, .runfl = 0, .flname = 0, .runflname = 0, .lex = lexeminit(),  ##__VA_ARGS__}

// ------------------------------------------ Utilities -------------------------------------------

// --------------------------------------- AUXILLARY PROCS -------------------------------------------

extern int                 proc_quit(Runtimedata *rt);
extern int                 proc_help(Runtimedata *rt);
extern int                 proc_techdump(Runtimedata *rt);

// ------------------------------------------- PROCS -------------------------------------------------

extern int                 proc_load(Runtimedata *rt);
extern int                 proc_print(Runtimedata *rt);
extern int                 proc_run(Runtimedata *rt);
extern int                 proc_save(Runtimedata *rt);
extern int                 proc_clear(Runtimedata *rt);
extern int                 proc_par(Runtimedata *rt);

// ------------------------------- Contructor/destructor -----------------------------------------------

extern void                freeRuntimedata(Runtimedata *rt);
extern Runtimedata         initRuntimedata(const char *restrict flname, const char *restrict runflname);

#endif /* !_CORE_IMPL_H */
