#include "dcl.h"
#include "fs.h"
#include "error.h"

ParseItem               ParseItemInit(int cnt){
    ParseItem t = {.curr = TokenInit(100), .name = fsinit(cnt), .res = fsinit(cnt), .datatype = fsinit(cnt) };
    return t;
}

void                    ParseItemFree(ParseItem *p){
    fsfreeall(&p->datatype, &p->curr.value, &p->res, &p->name);
}

void                    dcl(ParseItem *item){
    logenter("res [%s]", fsstr(item->res) );
    int ns = 0;
    while (gettoken(&item->curr) == (toktype) '*')
        ns++;
    dirdcl(item);
    while (ns-- > 0)
        fs_catstr(&item->res, " pointer to ");
    logret(0, "res [%s]", fsstr(item->res) );
}

void                    dirdcl(ParseItem *item){
    logenter("res [%s], tok [%s]:[%s]", fsstr(item->res), toktype_str(item->curr.typ), fsstr(item->curr.value) );
    if (item->curr.typ == '('){
        dcl(item);
        if (item->curr.typ != ')'){
            fprintf(stderr, "Error: missign ')' at %d:%d\n", item->curr.str, item->curr.col);
            userraiseint(101, "Error: missign ')' at %d:%d\n", item->curr.str, item->curr.col);
        }
    }
    else if (item->curr.typ == NAME)
        fscpy(item->name, item->curr.value);
    else {
        fprintf(stderr, "Expected name or dlc() at %d:%d\n", item->curr.str, item->curr.col);
        userraiseint(101, "Expected name or dlc() at %d:%d\n", item->curr.str, item->curr.col);
    }
    while ( (item->curr.typ = gettoken(&item->curr) ) == PARENS || item->curr.typ == BRACKETS)
        if (item->curr.typ == PARENS)
            fscatstr(item->res, " function returning");
        else {
            fscatstr(item->res, " array");
            fscat(item->res, item->curr.value);
            fscatstr(item->res, " of");
        }
    logret(0, "res [%s], tok [%s]:[%s]", fsstr(item->res), toktype_str(item->curr.typ), fsstr(item->curr.value) );
}

