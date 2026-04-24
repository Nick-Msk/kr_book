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
            const char *tp = lexem_str(l);
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
    if (ctx->mfr){
        if (meof(ctx->mfr) )
            printf("Read file is EOF\n");
        else
            printf("Read file is NOT EOF\n");
        return logsimpleret(1, "Ok");
    }
    else
        fprintf(stderr,  "Read file isn't open");
    return logsimpleerr(-1, "Not opened");
}

int                 proc_error(Context *ctx){
    if (ctx->mfw){
        if (merror(ctx->mfr) )
            printf("Write file in error state\n");
        else
            printf("Write file not in error state\n");
        return logsimpleret(1, "Ok");
    } else
        fprintf(stderr,  "Write file isn't open");
    return logsimpleerr(-1, "Not opened");
}

int                 proc_getpos(Context *ctx){
    Lexem *l = ctx->lex;
    if (getlexem(l, false) ){
        if (l->typ == LEXEM_WORD){
            const char *tp = lexem_str(l);
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
            const char *tp = lexem_str(l);
            if (strcmp(tp, "read") == 0){    // close read-associated file
                printf("Read fileno %d\n", mfileno(ctx->mfr) );
                return logsimpleret(1, "r fileno ok");
            } else if (strcmp(tp, "write") == 0){   // write-associated file
                printf("Write fileno %d\n", mfileno(ctx->mfr) );
                return logsimpleret(1, "w fileno ok");
            } else
                fprintf(stderr, "Type of file must be read or write %s\n", tp);
        } else
            fprintf(stderr, "Must be word (%s)\n", Lexemtype_str(l->typ) );
    } else
        fprintf(stderr, "Out of input\n");
    return logsimpleerr(-1, "Unable to get file no");
}

// read amount of data
int                 proc_read(Context *ctx){
    if (!ctx->mfr)
        fprintf(stderr, "Read file isn't open\n");
    else {
        Lexem *l = ctx->lex;
        if (getlexem(l, false) ){
            if (l->typ == LEXEM_INT){
                const char *tp = lexem_str(l);
                int         sz = atoi(tp), c = 0;
                if (sz > 0) {
                    printf("From %4d-%4d:", mgetpos(ctx->mfr), sz);
                    for (int i = 0; c != EOF && i < sz; i++)
                        putchar(c = mgetc(ctx->mfr) );
                    printf("\n");
                    if (c == EOF)
                        printf("EOF detected\n");
                    return logsimpleret(1, "Read %d ok", sz);
                } else
                    fprintf(stderr, "amount must be positive (%d)\n", sz);
            } else
                fprintf(stderr, "Must be int (%s)\n", Lexemtype_str(l->typ) );
        } else
            fprintf(stderr, "Out of input\n");
    }
    return logsimpleerr(-1, "Unable to read");
}
// write <peace of data>
int                 proc_write(Context *ctx){
    if (!ctx->mfw)
        fprintf(stderr, "Write file isn't open\n");
    else {
        Lexem *l = ctx->lex;
        if (getlexem(l, false) ){
            if (l->typ == LEXEM_WORD){
                const char *tp = lexem_str(l);
                int         sz = strlen(tp), c = 0;     // REMOVE THAT STRLEN TODO:
                    printf("Write %4d-%4d:", mgetpos(ctx->mfw), sz);
                    for (int i = 0; c != EOF && i < sz; i++)
                        mputc(tp[i], ctx->mfw);
                    return logsimpleret(1, "Read %d ok", sz);
            } else
                fprintf(stderr, "Must be word (%s)\n", Lexemtype_str(l->typ) );
        } else
            fprintf(stderr, "Out of input\n");
    }
    return logsimpleerr(-1, "Unable to read");
}

// <read/write> pos
int                 proc_seek(Context *ctx){ 
    Lexem *l = ctx->lex;
    if (getlexem(l, false) ){
        if (l->typ == LEXEM_WORD){
            const char *tp = lexem_str(l);
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

