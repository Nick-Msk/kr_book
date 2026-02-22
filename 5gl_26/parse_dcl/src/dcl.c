#include "dcl.h"
#include "fs.h"

void            dcl(fs *restrict res, fs *restrict name, Token *restrict curr){
    logenter("res [%s]", fsstr(*res) );
    int ns = 0;
    while (gettoken(curr) == (toktype) '*')
        ns++;
    dirdcl(res, name, curr);
    while (ns-- > 0)
        fs_catstr(res, " pointer to ");
    logret(0, "res [%s]", fsstr(*res) );
}


void            dirdcl(fs *restrict res, fs *restrict name, Token *restrict curr){
    logenter("res [%s], tok [%s]:[%s]", fsstr(*res), toktype_str(curr->typ), fsstr(curr->value) );
    if (curr->typ == '('){
        dcl(res, name, curr);
        if (curr->typ != ')')
            fprintf(stderr, "Error: missign ')'\n");
    }
    else if (curr->typ == NAME)
        fs_cpy(name, curr->value);
    else
        fprintf(stderr, "Expected name or dlc()\n");
    while ( (curr->typ = gettoken(curr) ) == PARENS || curr->typ == BRACKETS)
        if (curr->typ == PARENS)
            fs_catstr(res, " function returning");
        else {
            fs_catstr(res, " array");
            fs_cat(res, curr->value);
            fs_catstr(res, " of");
        }
    logret(0, "res [%s], tok [%s]:[%s]", fsstr(*res), toktype_str(curr->typ), fsstr(curr->value) );
}

