#include <stdio.h>

#include "proc_actions.h"
#include "checker.h"
#include "files.p8.5.h"
#include "getword.h"

static const char               mdir[] = "mdir/";

// ------------------------------------------ Utilities -------------------------------------------

int                             Context_techfprint(FILE *restrict out, const Context *restrict ctx){
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
int                             proc_quit(Context *ctx){
    ctx->quit = true;
    return 1;
}

int                             proc_help(Context *ctx){
    Command *cm = ctx->cmds;
    while (cm->desc){
        printf("%s\t\t: %s\n", cm->name, cm->desc);
        cm++;
    }
    return 1;
}

int                             proc_techprint(Context *ctx){
    // just technical print
    Context_techprint(ctx);
    return 1;
}

// ------------------------------------------- MFILE PROCS --------------------------------------------

// create filename
int                             proc_create(Context *ctx){
    logsimple("ctx %p, lex %p", ctx, ctx ? ctx->lex : 0);
    Lexem *l = ctx->lex;
    if (getlexem(l, false) )
        if (l->typ == LEXEM_WORD){
            fsrevcatstr(l->str, mdir); // eg. mdir/abc.txt
            MFILE      *mf;
            const char *fname = lexem_str(l);
            if ( (mf = mopen(fname, "w") ) == 0){
                fprintf(stderr, "Unable to create %s\n", fname);
            } else {
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

// open filename
int                             proc_open(Context *ctx){
    logsimple("ctx %p, lex %p", ctx, ctx ? ctx->lex : 0);
    Lexem *l = ctx->lex;
    if (getlexem(l, false) )
        if (l->typ == LEXEM_WORD){
            fsrevcatstr(l->str, mdir); // eg. mdir/abc.txt
            MFILE      *mf;
            const char *fname = lexem_str(l);
            if ( (mf = mopen(fname, "w") ) == 0){
                fprintf(stderr, "Unable to create %s\n", fname);
            } else {
                printf("File %s is opened for write\n", fname);
                ctx->mfw = mf;       // setup ctx;
                return logsimpleret(1, "File %s  opened for write", fname);
            }
        } else
            fprintf(stderr, "Word expected but (%s)", Lexemtype_str(ctx->lex->typ) );
    else
        fprintf(stderr, "Out of input\n");
    return logsimpleerr(-1, "Failed to open");  // failed
}
// close read/write
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
        } else
            fprintf(stderr, "Must be word (%s)\n", Lexemtype_str(l->typ) );
    } else
        fprintf(stderr, "Out of input\n");
    return logsimpleerr(-1, "Failed to close");  // failed
}

// only for read file
int                 proc_eof(Context *ctx){
    if (ctx->mfr)
        if (mfeof(ctx->mfr) )
            printf("Read file is EOF");
        else
            printf("Read file is NOT EOF");
    else {
        fprintf(stderr,  "Read file isn't open");
        return -1;
    }
    return 1;
}

int                 proc_getpos(Context *ctx){
    Lexem *l = ctx->lex;
    if (getlexem(l, false) ){
        if (l->typ == LEXEM_WORD){
            const char *tp = fsstr(l->str);
            if (strcmp(tp, "read") == 0){    // close read-associated file
                mgetpos(ctx->mfr);
            } else if (strcmp(tp, "write") == 0){   // write-associated file
                mgetpos(ctx->mfw);
            } else
                fprintf(stderr, "Type of file must be read or write %s\n", tp);
        } else
            fprintf(stderr, "Must be word (%s)\n", Lexemtype_str(l->typ) );
    } else
        fprintf(stderr, "Out of input\n");
    return logsimpleerr(-1, "Unable to obtain pos");
}

int                 proc_fileno(Context *ctx){
    Lexem *l = ctx->lex;
    if (getlexem(l, false) ){
        if (l->typ == LEXEM_WORD){
            const char *tp = fsstr(l->str);
            if (strcmp(tp, "read") == 0){    // close read-associated file
                mgetpos(ctx->mfr);
            } else if (strcmp(tp, "write") == 0){   // write-associated file
                mgetpos(ctx->mfw);
            } else
                fprintf(stderr, "Type of file must be read or write %s\n", tp);
        } else
            fprintf(stderr, "Must be word (%s)\n", Lexemtype_str(l->typ) );
    } else
        fprintf(stderr, "Out of input\n");
    return logsimpleerr(-1, "Unable to get file no");
}
