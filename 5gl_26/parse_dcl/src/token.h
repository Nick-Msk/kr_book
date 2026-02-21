#ifndef _TOKEN_H
#define _TOKEN_H

typedef enum {
    NAME, PARENS, BRACKETS, TOKEOF = EOF
} toktype;

typedef struct Token {
    toktype  typ;
    fs       value;
} Token;

toktype            gettoken(Token *t);

#endif /* !_TOKEN_H */
