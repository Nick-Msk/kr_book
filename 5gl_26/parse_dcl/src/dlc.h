#ifndef _DLC_H
#define _DLC_H

#include <stdio.h>
#include "token.h"

Token            dlc(fs *res, Token *curr);
Token            dirdcl(fs *res, Token *curr);

#endif /* !_DLC_H */
