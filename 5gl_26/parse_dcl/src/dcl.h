#ifndef _DLC_H
#define _DLC_H

#include <stdio.h>
#include "token.h"

typedef struct {
    Token   curr;
    fs      name;
    fs      res;
    fs      datatype;
} ParseItem;

extern void             dcl(ParseItem *item);
extern void             dirdcl(ParseItem *item);
extern ParseItem        ParseItemInit(int cnt);
extern void             ParseItemFree(ParseItem *p);

// printers
extern int              ParseItemfprintf(FILE *restrict f, ParseItem *restrict p, const char *restrict name);

static inline int       ParseItemprintf(ParseItem *restrict p, const char *restrict name){
    return ParseItemfprintf(stdout, p, name);
}

#define                 ParseItemfprint(out, p) ParseItemfprintf((out), &(p), #p)
#define                 ParseItemprint(p) ParseItemprintf(&(p), #p)

#endif /* !_DLC_H */
