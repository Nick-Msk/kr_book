#ifndef _PROC_ACTIONS_H
#define _PROC_ACTIONS_H

#include <stdio.h>
#include "bool.h"
#include "getword.h"
#include "files.p8.5.h"
#include "command_executor.h"

typedef struct Runtimedata {
    Lexem       *lex;
    MFILE       *mfr;   // read
    MFILE       *mfw;   // write
    bool        quit;   // flag for quit
    Command    *cmds;   // link to all commands
    // ....
} Runtimedata;
#define             RuntimedataInit(...) (Runtimedata) {.lex = 0, .mfr = 0, .mfw = 0, .quit = false,  ##__VA_ARGS__}

// ------------------------------------------ Utilities -------------------------------------------

extern int                 Runtimedata_techfprint(FILE *restrict out, const Runtimedata *restrict ctx);

static inline int          Runtimedata_techprint(const Runtimedata *ctx){
    return Runtimedata_techfprint(stdout, ctx);
}

// --------------------------------------- AUXILLARY PROCS -------------------------------------------

extern int                 proc_quit(Runtimedata *ctx);
extern int                 proc_help(Runtimedata *ctx);
extern int                 proc_techprint(Runtimedata *ctx);

// ------------------------------------------- MFILE PROCS --------------------------------------------
extern int                 proc_create(Runtimedata *ctx);
extern int                 proc_open(Runtimedata *ctx);
extern int                 proc_close(Runtimedata *ctx);
extern int                 proc_read(Runtimedata *ctx);
extern int                 proc_write(Runtimedata *ctx);
extern int                 proc_eof(Runtimedata *ctx);
extern int                 proc_error(Runtimedata *ctx);
extern int                 proc_getpos(Runtimedata *ctx);
extern int                 proc_fileno(Runtimedata *ctx);
extern int                 proc_seek(Runtimedata *ctx);

#endif /* !_PROC_ACTIONS_H */
