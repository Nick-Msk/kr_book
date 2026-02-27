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

int                     ParseItemfprintf(FILE *restrict f, ParseItem *restrict p, const char *restrict name){
    int     res = 0;
    if (p){
        res += fprintf(f, "ParseItem %s: ", name);
        res += Tokenfprint(f, p->curr);
        res += fsfprint(f, p->name) + fsfprint(f, p->res) + fsfprint(f, p->datatype);
        res += fprintf(f, "\n");
    }
    return res;
}

void                     parse(void){
    ParseItem item = ParseItemInit(100);
    while (gettoken(&item.curr) != TOKEOF){
        if (item.curr.typ == NAME)
            fscpy(item.datatype, item.curr.value);  // just copy from one fs to another
        else {
            fprintf(stderr, "should be datatype! but not %s at %d:%d", toktype_str(item.curr.typ), item.curr.str, item.curr.col);
            ParseItemFree(&item);
            userraiseint(101, "should be datatype! but not %s", toktype_str(item.curr.typ) );
        }
        //fsclone(t.value); // This is create the new fs!
        fsend(item.res, 0);
        dcl(&item);
        if (item.curr.typ != '\n'){
            fprintf(stderr, "SYntax error at %d:%d\n", item.curr.str, item.curr.col);
            ParseItemprint(item);
            userraiseint(101, "SYntax error at %d:%d\n", item.curr.str, item.curr.col);
        }
        printf("%s: %s %s\n",  fsstr(item.name), fsstr(item.res), fsstr(item.datatype) );
    }
    ParseItemFree(&item);
    return; // empty for now
}

void                    dcl(ParseItem *item){
    bool    pointer = false;
    if (gettoken(&item->curr) == (toktype) '*')
        pointer++;
    dirdcl(&item);
    if (pointer)
        fscatstr(item->res, " pointer to");
}

void                    dirdcl(ParseItem *item){
    
}

