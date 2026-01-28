#ifndef _GETOP_H
#define _GETOP_H

#include "common.h"

// ---------------------------------------------------------------------------------
// --------------------------- Public GETOP API ----------------------------------
// ---------------------------------------------------------------------------------

// ----------- CONSTANTS AND GLOBALS ---------------

// ------------------- TYPES -----------------------

typedef enum {
    LEXIC_NO_OP = 0,
    LEXIC_NUMBER = '0',
    LEXIC_PLUS = '+',
    LEXIC_MINUS = '-',
    LEXIC_MUL = '*',
    LEXIC_DIV = '/',
    LEXIC_MOD = '%',
    LEXIC_POW = '^',
    LEXIC_PRINT = 'p',
    LEXIC_HELP = 'h',
    LEXIC_EXCH = 'e',
    LEXIC_PUSHSAME = 'd',
    LEXIC_QUIT = 'q',
    LEXIC_CLEAR = 'c',
    LEXIC_REMOVE = 'r',
    LEXIC_SIN = 'z' + 1, // TODO
    LEXIC_COS,
    LEXIC_TAN,
    LEXIC_VAR = ':',
    LEXIC_ASSIGNMENT = '='
} LexicOper;

static inline const char        *LexicOperName(LexicOper t){
    switch (t){
        CASE_RETURN(LEXIC_NO_OP);
        CASE_RETURN(LEXIC_NUMBER);
        CASE_RETURN(LEXIC_PLUS);
        CASE_RETURN(LEXIC_MINUS);
        CASE_RETURN(LEXIC_MUL);
        CASE_RETURN(LEXIC_DIV);
        CASE_RETURN(LEXIC_MOD);
        CASE_RETURN(LEXIC_POW);
        CASE_RETURN(LEXIC_PRINT);
        CASE_RETURN(LEXIC_HELP);
        CASE_RETURN(LEXIC_EXCH);
        CASE_RETURN(LEXIC_PUSHSAME);
        CASE_RETURN(LEXIC_QUIT);
        CASE_RETURN(LEXIC_CLEAR);
        CASE_RETURN(LEXIC_REMOVE);
        CASE_RETURN(LEXIC_SIN);
        CASE_RETURN(LEXIC_COS);
        CASE_RETURN(LEXIC_TAN);
        CASE_RETURN(LEXIC_VAR);
        CASE_RETURN(LEXIC_ASSIGNMENT);
        default: return "";
    }
}

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

// TODO:

// -------------- ACCESS AND MODIFICATION ----------

extern LexicOper        lexic_getop(char *s, int sz);
extern void             lexic_clear(void);

// ----------------- PRINTERS ----------------------

extern int              lexic_fprint(FILE *f);

static inline int       lexic_print(void){
    return lexic_fprint(stdout);
}

// ------------------ ETC. -------------------------

#endif /* !_GETOP_H */
