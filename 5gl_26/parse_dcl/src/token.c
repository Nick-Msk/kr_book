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
static              char buf[2] = {'\0', '\0'};

const char       *toktype_str(toktype t){
    switch (t) {
        CASE_RETURN(NAME);
        CASE_RETURN(PARENS);
        CASE_RETURN(TOKEOF);
        CASE_RETURN(BRACKETS);
        default:
            buf[0] = (char) t;
            return buf;
    }
}

toktype           gettoken(Token *t){
    logenter("...");
    int   c, p = 0;
    while ( (c = getch()) == ' ' || c == '\t')
        ;
    logauto((char)c);
    if (c == '(') {
        if ( (c = getch()) == ')'){
            fscatstr(t->value, "()");
            t->typ = PARENS;
        } else {
            ungetch(c);
            t->typ = '(';
        }
    } else if (c == '['){
            p = 0;
            for (elem(t->value, p++) = c; (elem(t->value, p++) = getch() ) != ']'; )  // OMG
                ;
            fsend(t->value, p); // impossible just to to elem() = '\0';
            t->typ = BRACKETS;
    } else if (isalpha(c) ) {
        p = 0;
        for (elem(t->value, p++) = c; isalnum(c = getch() ); )  // OMG
            elem(t->value, p++) = c;
        fsend(t->value, p); // set '\0' and len
        ungetch(c);
        t->typ = NAME;
    } else
        t->typ = (char) c;

    return logret(t->typ, "Tok [%s] value [%s]", toktype_str(t->typ), fsstr(t->value) );
}

