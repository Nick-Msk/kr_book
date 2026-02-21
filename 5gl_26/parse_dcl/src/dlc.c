#include "dlc.h"
#include "fs.h"

Token            dlc(fs *res, Token *curr){
    int ns = 0;
    while (gettoken(curr) == (toktype) '*')
        ns++;
    dirdlc(res, curr);
    while (ns-- > 0)
        fs_catstr(res, " pointer to ");
}


Token            dirdcl(fs *res, Token *curr){


}
