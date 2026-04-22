#ifndef _GETWORD_H
#define _GETWORD_H

#include "fs.h"
#include "bool.h"
#include "checker.h"

extern fs                   getword(fs str, bool sens, bool comments, bool get_newline);
// STR MUST BE init with alloc flag
static inline bool                 getsimpleword(fs *str){
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

static inline const char   *lexemstr(Lexem l){
    return fsstr(l.str);
}
// pointer version
static inline const char   *lexem_str(const Lexem *l){
    invraise(l != 0, "Wrong input");
    return l->str.v;
}

static inline int           lexem_techfprint(FILE *restrict out, const Lexem *restrict l){
    invraise(l != 0, "Wrong input");
    int     cnt = 0;
    if (out){
        cnt += fprintf(out, "LEXEM: %s:[", Lexemtype_str(l->typ));
        cnt += fs_techfprint(out, &l->str, "");
        cnt += fprintf(out, "]\n");
    }
    return cnt;
}

static inline int           lexem_techprint(const Lexem *l){
    return lexem_techfprint(stdout, l);
}

static inline void          lexem_free(Lexem *l){
    invraise(l != 0, "Wrong input");
    fsfree(l->str);
    l->typ = LEXEM_UNK;
}

#define                     lexeminit(...) {.str = FS(), .typ = LEXEM_UNK }
#define                     lexemfree(l) lexem_free( &(l) )

// any lexem, word or number
extern bool                 getlexem(Lexem *lex, bool ign_comments);

// invariant: lex != 0 && str != 0
static inline bool          lexem_eq(const Lexem *restrict lex, const char *restrict str, int len){
    invraise(lex != 0 && str != 0, "Wrong input");
    return strncmp(lex->str.v, str, len) == 0;
}
// fs port
static inline bool          lexem_fseq(const Lexem *restrict lex, const fs *restrict str, int len){
    invraise(lex != 0 && str != 0, "Wrong input");
    return fs_ncmp(&lex->str, str, len) == 0;
}

#endif /* ! _GETWORD_H */

