#ifndef _TOKEN_H
#define _TOKEN_H

#include "fs.h"

typedef enum {
    NAME = 0, PARENS, BRACKETS, TOKEOF = EOF
} toktype;

typedef struct Token {
    toktype  typ;
    fs       value;
    int      str, col, laststrsz;
} Token;

static inline Token       TokenInit(int sz){
    Token t = (Token) {.str = 1, .col = 1, .laststrsz = 0, .typ = NAME, .value = fsinit(sz) };
    return t;
}

extern toktype            gettoken(Token *t);

extern const char        *toktype_str(toktype t);

static inline int         Tokenfprintf(FILE *restrict f, Token *restrict t, const char *restrict name){
    int     res = 0;
    if (t){
        res += fprintf(f, "%s: ", name);
        res += fsfprint(f, t->value);
        res += fprintf(f, "typ: %s", toktype_str(t->typ) );
    }
    return res;
}

static inline int         Tokenprintf(Token *restrict t, const char *restrict name){
    return Tokenfprintf(stdout, t, name);
}

#define                  Tokenfprint(f, t) Tokenfprintf( (f), &(t), #t)

#define                  Tokenprint(t) Tokenfprintf( &(t), #t)

#endif /* !_TOKEN_H */
