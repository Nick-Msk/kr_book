#ifndef _DLC_H
#define _DLC_H

#include <stdio.h>
#include "token.h"

void             dcl(fs *restrict res, fs *restrict name, Token *curr);
void             dirdcl(fs *restrict res, fs *restrict name, Token *restrict curr);

#endif /* !_DLC_H */
