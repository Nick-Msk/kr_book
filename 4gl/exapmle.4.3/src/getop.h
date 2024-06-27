#ifndef _GETOP_H_
#define _GETOP_H_

#include "bool.h"

#define         MAXNAME     32

typedef enum
    {NUMBER = 100, PLUS, MINUS, DIV, MUL, MOD, SIN, EXP, POW,
            HELP, QUIT, PRINT_STACK, CLEAR, DOU, EXCH,
            GET, SET, UNSET, ARR,
            UNKNOWN = 9999} Ops;

typedef struct {
    Ops         type;
    const char  str[MAXNAME];    // fills for some kind of opers (GET, SET, UNSET)
    union {
        double      dval;   // fills only for number
        int         ival;   // array number!
    };
} Oper;

Oper               getop(void);

#endif /* _GETOP_H_ */

