#ifndef _GETWORD_H
#define _GETWORD_H

#include "fs.h"
#include "bool.h"

extern fs                   getword(fs str, bool sens, bool comments, bool get_newline);
// STR MUST BE init with alloc flag
static bool                 getsimpleword(fs *str){
    *str = getword(*str, false, false, false);
    return !fsisnull(*str);
}

// LEXEM_CMD for \<cmd>
typedef enum { LEXEM_UNK = -1, LEXEM_WORD, LEXEM_INT, LEXEM_FLOAT, LEXEM_SYM, LEXEM_CMD } Lexemtype;

static inline const char   *Lexemtype_str(Lexemtype typ){
    switch (typ){
        CASE_RETURN(LEXEM_UNK);
        CASE_RETURN(LEXEM_WORD);
        CASE_RETURN(LEXEM_INT);
        CASE_RETURN(LEXEM_FLOAT);
        CASE_RETURN(LEXEM_SYM);
        CASE_RETURN(LEXEM_CMD);
        default: return "";
    }
}

typedef struct Lexem {
    fs          str;
    Lexemtype   typ;
} Lexem;

static inline const char *  Lexemstr(Lexem l){
    return fsstr(l.str);
}

static void                 Lexem_free(Lexem *l){
    fsfree(l->str);
    l->typ = LEXEM_UNK;
}

#define                     Lexeminit(...) {.str = FS(), .typ = LEXEM_UNK }
#define                     Lexemfree(l) Lexem_free( &(l) )

// any lexem, word or number
extern bool                 getlexem(Lexem *lex, bool ign_comments);

#endif /* ! _GETWORD_H */

