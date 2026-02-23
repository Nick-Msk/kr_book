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

#endif /* !_DLC_H */
