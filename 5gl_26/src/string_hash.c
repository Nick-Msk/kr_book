#include <stdlib.h>
#include <stdio.h>

#include "log.h"
#include "string_hash.h"
#include "numeric_ops.h"
#include "error.h"
#include "common.h"

static const int                        G_STRINGLIST_NEWLINE = 10;

// --------------------------------------- Utility ------------------------------------

// np != 0
static void                 freeelement(stringlist *np){
    free(np->name);
    free(np->defn);
    free(np);
}
// list != 0
static void                freelist(stringlist *list){
    if (list->next)
        freelist(list->next);
    freeelement(list);
}


static inline stringlist **alloclists(int cnt){
    return (stringlist **) malloc(sizeof(stringlist) * cnt);
}

static inline stringlist  *alloclist(void){
    return (stringlist *) alloclists(1);
}

static unsigned            hash_simple(const char *str, unsigned max){
    const char *s = str;
    unsigned long hashval = 0L;
    for (; *s != '\0'; s++)
        hashval = *s + 31 * hashval;
    hashval %= max;
    return logsimpleret( (unsigned) hashval, "Generated %u for [%s]", (unsigned) hashval, str);
}
// parameter aren't null
static int                 fprintstrlist(FILE *restrict out, const stringlist *restrict lst){
    int     cnt = 0, i = 0;
    while(lst){
        cnt += fprintf(out, "[%s:%s] ", lst->name, lst->defn);
        if (i % G_STRINGLIST_NEWLINE == G_STRINGLIST_NEWLINE - 1)
            cnt += fprintf(out, "\n");
        lst = lst->next, i++;
    }
    if (i > 0 && i % G_STRINGLIST_NEWLINE != 0)
        cnt += fprintf(out, "\n");
    return cnt;
};

// ----------------------------------------- API --------------------------------------
// num will be upscaled to simple number
stringhash                 strhash_create(int num, StringHashType typ){

    if (int_notin(typ, HASH_SIMPLE) )
        userraiseint(ERR_WRONG_PARAMETER, "Unknown hash type %d", typ);

    num = calc_next_prime(num);
    logsimple("Will trying to alloc %d", num);
    stringlist **t = alloclists(num);

    if (!t)
        userraiseint(ERR_UNABLE_ALLOCATE, "Unable to alloc %lu", num * sizeof(stringlist) );

    stringhash tmp = {.tab = t, .sz = num, .typ = typ};
    return tmp;
}

void                       strhash_clear(stringhash *hash){
    for (int i = 0; i < hash->sz; i++)
        if (hash->tab[i]){
            freelist(hash->tab[i]);
            hash->tab[i] = 0;
        }
}

void                       strhash_free(stringhash *hash){
    strhash_clear(hash);
    hash->sz = 0;
    free(hash->tab);
    hash->tab = 0;
    // hash NOT freed here! It's not allocated
}

unsigned                    strhash(const stringhash *restrict hashtab, const char *restrict str){

    unsigned ret = 0;
    switch (hashtab->typ){
        case HASH_SIMPLE:
            ret = hash_simple(str, hashtab->sz);        // what if hash function is part of stringhash directly? TODO:
        break;
    }
    return ret;
}

stringlist                 *strhash_lookup(const stringhash *restrict hashtab, const char *restrict str){

    stringlist *np;
    // iterator ???
    for (np = hashtab->tab[strhash(hashtab, str)]; np != 0; np = np->next)
        if (strcmp(np->name, str) == 0)
            return np;
    return 0;
}

stringlist                 *strhash_install(stringhash *restrict hashtab, const char *restrict name, const char *restrict defn){
    unsigned    hashval;
    stringlist *np;
    if ( (np = strhash_lookup(hashtab, name) ) == 0){
        np = alloclist();
        if (np == 0 || (np->name = strdup(name) ) == 0)
            logsimpleret(0, "Unable to create strlist or duplicate name");
        hashval = strhash(hashtab, name);
        logsimple("---- %p", hashtab->tab[hashval]);
        np->next = hashtab->tab[hashval];
        hashtab->tab[hashval] = np;  // mount
    } else  // already exists in the table
        free(np->defn);
    if ( (np->defn = strdup(defn) ) == 0)
        return logsimpleerr( (stringlist *) 0, "Unable to dup [%s]", defn);
    return logsimpleret(np, "Installed [%s] - [%s]", name, defn);
}

bool                        strhash_undef(stringhash *restrict hashtab, const char *restrict name){
    logenter("%s", name);
    bool            ret = false;
    stringlist     *np, *nprev = 0;
    int             pos;
    for (np = hashtab->tab[pos = strhash(hashtab, name)]; np != 0; nprev = np, np = np->next){
        if (strcmp(np->name, name) == 0){
            if (nprev == 0) // first iteration
                hashtab->tab[pos] = np->next;
            else
                nprev->next = np->next;
            freeelement(np);
            return logret(true, "Unmapped");
        }
    }
    return logret(ret, "Not found");
}

int                         strhash_fprint(FILE *restrict out, const stringhash *restrict hashtab){
    logsimple("......");
    int     cnt = 0;
    if (out && hashtab){
        for (int i = 0; i < hashtab->sz; i++)
            if (hashtab->tab[i]){
                printf("%4d => ", i);
                cnt += fprintstrlist(out, hashtab->tab[i]);
            }
    }
    return cnt;
}

int                         strhash_cnt(const stringhash *restrict hashtab){
    int cnt = 0;
    stringlist *np;
    for (int i = 0; i < hashtab->sz; i++){
        np = hashtab->tab[i];
        while (np)
            np = np->next, cnt++;
    }
    return cnt;
}

