#ifndef _TOKEN_H
#define _TOKEN_H

#include "fs.h"

typedef enum {
    NAME = 0, PARENS, BRACKETS, TOKEOF = EOF
} toktype;

typedef struct Token {
    toktype  typ;
    fs       value;
} Token;

extern toktype            gettoken(Token *t);

extern const char        *toktype_str(toktype t);

#endif /* !_TOKEN_H */
