#ifndef _STRING_HASH_H
#define _STRING_HASH_H

#include "fs.h"

// --------------------------------------------------------------------------------------
// ----------------------------- Public fs Hash API -------------------------------------
// --------------------------------------------------------------------------------------

// TODO: think about fs instead of char *
typedef struct stringlist {
    struct stringlist      *next;
    char                   *name;
    char                   *defn;
} stringlist;

typedef enum StringHashType { HASH_SIMPLE = 0 } StringHashType;

typedef struct stringhash {
    int             sz;
    StringHashType  typ;
    stringlist    **tab;
} stringhash;


// num will be upscaled to simple number
extern stringhash          strhash_create(int num, StringHashType typ);

extern void                strhash_free(stringhash *hashtab);
#define                    strhashfree(h) strhash_free(&(h) )

extern void                strhash_clear(stringhash *hashtab);

extern unsigned            strhash(const stringhash *restrict hashtab, const char *restrict str);

extern stringlist         *strhash_lookup(const stringhash *restrict hashtab, const char *restrict str);
// fs port
static inline stringlist  *strhash_fslookup(const stringhash *restrict hashtab, const fs *restrict str){
    return strhash_lookup(hashtab, str->v);
}

extern stringlist         *strhash_install(stringhash *restrict hashtab,  const char *restrict str, const char *restrict val);
// fs port
static inline stringlist  *strhash_fsinstall(stringhash *restrict hashtab,  const fs *restrict str, const fs *restrict val){
    return strhash_install(hashtab, str->v, val->v);
}

extern bool                strhash_undef(stringhash *restrict hashtab, const char *restrict str);
// fs port
static inline bool         strhash_fsundef(stringhash *restrict hashtab, const fs *restrict str){
    return strhash_undef(hashtab, str->v);
}

extern int                 strhash_fprint(FILE *restrict out, const stringhash *restrict hashtab);
static inline int          strhash_print(const stringhash *hashtab){
    return strhash_fprint(stdout, hashtab);
}

extern int                 strhash_cnt(const stringhash *restrict hashtab);

#endif /* !_STRING_HASH_H */

