#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "string_hash.h"
#include "numeric_ops.h"

// REMOVE
typedef struct stringlist {
    struct fslist          *next;
    char                   *name;
    char                   *defn;
} stringlist;

typedef stringhash {
    int         sz;
    stringlist  *tab;
}

// --------------------------------------- Utility ------------------------------------

// list != 0
static void                freelist(stringlist *list){
    while (list->next){
        freelist(list->next);
        free(list->name);
        free(list->defn);
        free(list);
    }
}

static inline stringlist  *alloclist(int cnt){
    return (stringlist *) mallloc(sizeof(stringlist) * cnt);
}

static unsigned            hash_simple(const char *str, unsigned max){
    unsigned long hashval = 0L;
    for (; *s != '\0'; s++)
        hashval = *s + 31 * hashval;
    return (unsigned) hashval % hashtab->sz;
}

// ----------------------------------------- API --------------------------------------
// num will be upscaled to simple number
stringhash                 strhash_create(int num, StringHashType typ){
    num = calc_next_prime(num);
    stringlist *t = alloclist(num);
    if (!t)
        userraiseint(ERR_UNABLE_ALLOCATE, "Unable to alloc %d", num * sizeof(stringlist) );
    stringhash tmp = {.tab = t, .sz = num, .typ = typ};
    return tmp;
}

void                       strhash_free(stringhash hash){
    for (int i = 0; i < hash.sz; i++)
        freelist(hash.tab[i]);
}

unsigned                    strhash(const stringhash *restrict hashtab, const char *restrict str){
    unsigned ret = 0;
    switch (hashtab->typ){
        case HASH_SIMPLE:
            ret = hash_simple();        // what if hash function is part of stringhash directly? TODO:
        break;
        default:
            logsimple("Unknown hash type %d", typ);
        break;
    }
    return ret;
}

stringlist                 *strhash_lookup(const stringhash *restrict hashtab, const char *restrict str){
    TODO:
}

stringlist                 *strhash_install(const char *restrict str, const char *restrict val);

bool                        strhash_undef(const char *str);
