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
        default: return "";
    }
}

typedef struct Lexem {
    fs          str;
    Lexemtype   typ;
} Lexem;

static inline const char *  Lexem_str(Lexem l){
    return fsstr(l.str);
}

#define                     LexemInit(...) {.str = FS(), .typ = LEXEM_UNK }

// any lexem, word or number
extern bool                 getlexem(Lexem *lex, bool ign_comments);

#endif /* ! _GETWORD_H */

