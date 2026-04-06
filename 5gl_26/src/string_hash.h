#ifndef _STRING_HASH_H
#define _STRING_HASH_H

// --------------------------------------------------------------------------------------
// ----------------------------- Public fs Hash API -------------------------------------
// --------------------------------------------------------------------------------------

typedef struct stringlist {
    struct fslist          *next;
    char                   *name;
    char                   *defn;
} stringlist;

typedef stringhash {
    int         sz;
    stringlist  *tab;
}

// num will be upscaled to simple number
extern stringhash          strhash_create(int num);

extern unsigned            strhash(const char *str);

extern stringlist         *strhash_lookup(const char *str);

extern stringlist         *strhash_install(const char *restrict str, const char *restrict val);

#endif /* !_STRING_HASH_H */

