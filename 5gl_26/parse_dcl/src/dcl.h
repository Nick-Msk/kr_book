#ifndef _DLC_H
#define _DLC_H

#include <stdio.h>
#include "token.h"

void             dcl(fs *res, Token *curr);
void             dirdcl(fs *res, Token *curr);

#endif /* !_DLC_H */
