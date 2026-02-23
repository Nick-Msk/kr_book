#include "token.h"
#include "buffer.h"
#include "fs.h"
#include <ctype.h>

// #include "fs_iter.h"

/* typedef enum {
    NAME = 0, PARENS, BRACKETS, TOKEOF = EOF 
} toktype;

typedef struct Token {
    toktype  typ;
    fs       value;
} Token; */
static              char buf[3] = {'\0', '\0', '\0'};

const char       *toktype_str(toktype t){
    switch (t) {
        CASE_RETURN(NAME);
        CASE_RETURN(PARENS);
        CASE_RETURN(TOKEOF);
        CASE_RETURN(BRACKETS);
        default:
            if ( (char) t != '\n')
                buf[0] = (char) t, buf[1] = '\0';
            else
                buf[0] = '\\', buf[1] = 'n';
            return buf;
    }
}

static inline char      tgetch(Token *t){
    int c = getch();
    if (c != '\n')
        t->col++; // don't care about tab
    else {
        t->laststrsz = t->col;  // for WA
        t->col = 1;
        t->str++;
    }
    return c;
}

static inline void      tungetch(Token *t, int c){
    if (c != '\n'){
        if (t->col == 1)
            fprintf(stderr, "Positiion error (col == 1)\n");
        else
            t->col--;
    } else {
        if (t->str == 1)
            fprintf(stderr, "Positiion error (str == 1)\n");
        else {
            t->col = t->laststrsz;  // this is WA for the case when only 1 char can be returned
            t->str--;
        }
    }
    ungetch(c);
}

toktype           gettoken(Token *t){
    logenter("...");
    int     c, p = 0;
    while ( (c = tgetch(t)) == ' ' || c == '\t')
        ;
    //logauto((char)c);
    if (c == '(') {
        if ( (c = tgetch(t)) == ')'){
            fscatstr(t->value, "()");
            t->typ = PARENS;
        } else {
            tungetch(t, c);
            t->typ = '(';
        }
    } else if (c == '['){
            p = 0;
            for (elem(t->value, p++) = c; (elem(t->value, p++) = tgetch(t) ) != ']'; )  // OMG
                ;
            fsend(t->value, p); // impossible just to to elem() = '\0';
            t->typ = BRACKETS;
    } else if (isalpha(c) ) {
        p = 0;
        for (elem(t->value, p++) = c; isalnum(c = tgetch(t) ); )  // OMG
            elem(t->value, p++) = c;
        fsend(t->value, p); // set '\0' and len
        tungetch(t, c);
        t->typ = NAME;
    } else
        t->typ = (char) c;

    return logret(t->typ, "Tok [%s] value [%s]", toktype_str(t->typ), fsstr(t->value) );
}

