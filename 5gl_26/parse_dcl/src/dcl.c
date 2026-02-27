#include "dcl.h"
#include "fs.h"
#include "error.h"

static void             dcl(ParseItem *item);
static void             dirdcl(ParseItem *item);

static void             listoftypes(ParseItem *item){
    logenter("...");
    toktype  typ;
    bool     first_run = true;
    do {
        if ( (typ = gettoken(&item->curr) ) == TOK_NAME){
            if (first_run){
                fscatstr( item->res, " function (");
                first_run = false;
                fscat(item->res, item->curr.value);
            } else {
                fscatstr(item->res, ", ");
                fscat(item->res, item->curr.value);
            }
        } else if (first_run){
            logret(0, "tok = %s but not a name", toktype_str(typ));      // TODO: make logret/logerr return void normally
            return;     // not a name, no parameter type list 
        }
    } while ( (typ = gettoken(&item->curr)) == ',');
    if (typ != ')'){
        fprintf(stderr, ") is missgn after list of parameter at %d:%d", item->curr.str, item->curr.col);
        userraiseint(101, ") is missgn after list of parameter at %d:%d", item->curr.str, item->curr.col);
    }
    fscatstr(item->res, ") returning");
    //item->curr.typ = TOK_PARENS;   // for standard line
    logret(0, "typ %s", toktype_str(typ));
}

static void             dcl(ParseItem *item){
    logenter("res [%s]", fsstr(item->res) );
    int ns = 0;
    while (gettoken(&item->curr) == (toktype) '*')
        ns++;
    dirdcl(item);
    while (ns-- > 0)
        fs_catstr(&item->res, " pointer to ");
    logret(0, "res [%s]", fsstr(item->res) );
}

static void              dirdcl(ParseItem *item){
    logenter("res [%s], tok [%s]:[%s]", fsstr(item->res), toktype_str(item->curr.typ), fsstr(item->curr.value) );
    if (item->curr.typ == '('){
        dcl(item);
        if (item->curr.typ != ')'){
            fprintf(stderr, "Error: missign ')' at %d:%d\n", item->curr.str, item->curr.col);
            userraiseint(101, "Error: missign ')' at %d:%d\n", item->curr.str, item->curr.col);
        }
    }
    else if (item->curr.typ == TOK_NAME)
        fscpy(item->name, item->curr.value);
    else {
        fprintf(stderr, "Expected name or dlc() at %d:%d\n", item->curr.str, item->curr.col);
        userraiseint(101, "Expected name or dlc() at %d:%d\n", item->curr.str, item->curr.col);
    }
    while ( (item->curr.typ = gettoken(&item->curr) ) == TOK_PARENS || item->curr.typ == TOK_BRACKETS || item->curr.typ == '('){
        if (item->curr.typ == '(')
            listoftypes(item); //  gather  type <, type> )
        else if (item->curr.typ == TOK_PARENS)
            fscatstr(item->res, " function returning");
        else {
            fscatstr(item->res, " array");
            fscat(item->res, item->curr.value);
            fscatstr(item->res, " of");
        }
    }
    logret(0, "res [%s], tok [%s]:[%s]", fsstr(item->res), toktype_str(item->curr.typ), fsstr(item->curr.value) );
}

// --------------------------------------- API ------------------------------------------

ParseItem               ParseItemInit(int cnt){
    ParseItem t = {.curr = TokenInit(100), .name = fsinit(cnt), .res = fsinit(cnt), .datatype = fsinit(cnt), .listofargs = fsinit(cnt) };
    return t;
}

void                    ParseItemFree(ParseItem *p){
    fsfreeall(&p->datatype, &p->curr.value, &p->res, &p->name, &p->listofargs);
}

int                     ParseItemfprintf(FILE *restrict f, ParseItem *restrict p, const char *restrict name){
    int     res = 0;
    if (p){
        res += fprintf(f, "ParseItem %s: ", name);
        res += Tokenfprint(f, p->curr);
        //res += fsfprints(f, &p->name, &p->res, &p->datatype);   // print array of fs
        res += fsfprint(f, p->name) + fsfprint(f, p->res) + fsfprint(f, p->datatype) + fsfprint(f, p->listofargs);
        res += fprintf(f, "\n");
    }
    return res;
}

void                     parse(void){

    int         numline = 0;
    ParseItem   item = ParseItemInit(100);
    while (gettoken(&item.curr) != TOK_EOF){
        if (item.curr.typ == TOK_NAME)
            fscpy(item.datatype, item.curr.value);  // just copy from one fs to another
        else {
            fprintf(stderr, "should be datatype! but not %s at %d:%d", toktype_str(item.curr.typ), item.curr.str, item.curr.col);
            ParseItemFree(&item);
            userraiseint(101, "should be datatype! but not %s", toktype_str(item.curr.typ) );
        }
        //fsclone(t.value); // This is create the new fs!
        fsend(item.res, 0);
        fsend(item.listofargs, 0);
        dcl(&item);
        if (item.curr.typ != '\n'){
            fprintf(stderr, "SYntax error at %d:%d\n", item.curr.str, item.curr.col);
            ParseItemprint(item);
            userraiseint(101, "SYntax error at %d:%d\n", item.curr.str, item.curr.col);
        }
        printf("%d: %s: ", numline++, fsstr(item.name));
        //if (fslen(item.listofargs) > 0)
          //  printf("(%s) ", fsstr(item.listofargs) );
        printf("%s %s\n", fsstr(item.res), fsstr(item.datatype) );
    }
    ParseItemFree(&item);
    return; // empty for now
}

