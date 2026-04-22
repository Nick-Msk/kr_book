#include <stdio.h>

#include "proc_actions.h"
#include "checker.h"
#include "files.p8.5.h"
#include "getword.h"

// ------------------------------------------ Utilities -------------------------------------------

int                 Context_techfprint(FILE *restrict out, const Context *restrict ctx){
    invraise(ctx != 0, "Null context");
    int     cnt = 0;
    if (out){
        cnt += fprintf(out, "CONTEXT: quit %s, ", bool_str(ctx->quit) );
        if (ctx->mfr){
            cnt += fprintf(out, "MFR: ");
            cnt += mfile_techfprint(out, ctx->mfr, false);
        }
        if (ctx->mfw){
            cnt += fprintf(out, "MFW: ");
            cnt += mfile_techfprint(out, ctx->mfw, false);
        }
        if (ctx->lex){
            cnt += lexem_techfprint(out, ctx->lex);
        }
    }
    return logsimpleret(cnt, "Printed %d", cnt);
}

// --------------------------------------- AUXILLARY PROCS -------------------------------------------



// example of working procedures
// local
int                 proc_quit(Context *ctx){
    ctx->quit = true;
    return 1;
}

int                 proc_help(Context *ctx){
    Command *cm = ctx->cmds;
    while (cm->desc){
        printf("%s\t\t: %s\n", cm->name, cm->desc);
        cm++;
    }
    return 1;
}

int                 proc_techprint(Context *ctx){
    // just technical print
    Context_techprint(ctx);
    return 1;
}

// ------------------------------------------- MFILE PROCS --------------------------------------------

// LOCAL proc
int                 proc_create(Context *ctx){
    logsimple("ctx %p, lex %p", ctx, ctx ? ctx->lex : 0);
    Lexem *l = ctx->lex;
    if (getlexem(l, false) )
        if (l->typ == LEXEM_WORD){
            const char *fname = fsstr(l->str);
            MFILE      *mf;
            if ( (mf = mopen(fname, "w") ) == 0)
                fprintf(stderr, "Unable to create %s\n", fname);
            else {
                printf("File %s is opened for write\n", fname);
                ctx->mfw = mf;       // setup ctx;
                return logsimpleret(1, "File %s was created and opened for write", fname);
            }   
        } else
            fprintf(stderr, "Word expected but (%s)", Lexemtype_str(ctx->lex->typ) );
    else
        fprintf(stderr, "Out of input\n");
    return logsimpleerr(-1, "Failed to create");  // failed
}

// LOCAL proc
int                 proc_close(Context *ctx){
    Lexem *l = ctx->lex;
    if (getlexem(l, false) ){
        if (l->typ == LEXEM_WORD){
            const char *tp = fsstr(l->str);
            if (strcmp(tp, "read") == 0){    // close read-associated file
                mclose(ctx->mfr);
                ctx->mfr = 0;
            } else if (strcmp(tp, "write") == 0){   // write-associated file
                mclose(ctx->mfw);
                ctx->mfw = 0;
            } else {
                fprintf(stderr, "Type of file must be read or write, e.g. 'close read' but not %s\n", tp);
                return logsimpleret(-1, "Incorrect close type %s", tp);
            }
            printf("%s is closed", tp);
            return logsimpleret(1, "%s closed", tp);
        }
    } else
        fprintf(stderr, "Out of input\n");
    return logsimpleerr(-1, "Failed to close");  // failed
}


