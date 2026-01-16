#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


#include "log.h"
#include "common.h"
#include "checker.h"
#include "metric.h"

// TODO: 2 array.h
typedef struct {
    int     len;
    int     sz; // total size, > len + 1
    int    *v;
} IArray;

// array, but not IArray, because common for int and double
typedef enum ArrayFillType{
    ARRAY_NONE = 0,
    ARRAY_DESC,
    ARRAY_ACS,
    ARRAY_RND,
    ARRAY_ZERO
} ArrayFillType;

static inline const char        *ArrayFillTypeName(ArrayFillType t){
    switch (t){
        CASE_RETURN(ARRAY_NONE);
        CASE_RETURN(ARRAY_DESC);
        CASE_RETURN(ARRAY_ACS);
        CASE_RETURN(ARRAY_RND);
        CASE_RETURN(ARRAY_ZERO);
        default: return "";
    }
}


#define                 IArray_init(...) (IArray){.len = 0, .sz = 0, .v = 0}

// CREATE  and fill with method
// to array.h
IArray                  IArray_create(int cnt, ArrayFillType typ){
    logenter("cnt %d, typ %s", cnt, ArrayFillTypeName(typ));
    IArray      res = IArray_init();
    int         initval;

    res.sz    = round_up_2(cnt);   // 2^(x + 1)
    res.len   = cnt;
    res.v   = malloc(res.sz * sizeof(int));    // TODO: sysraise should be here!!!
    // fill
    switch (typ){
        case ARRAY_DESC:
            initval = 100 * res.len;   // hope it'll ne owerwelhm int
            int     dec_value = 10;          // for now!!! It'll be changed
            for (int i = 0; i < res.len; i++)
                res.v[i] = initval -= rndint(dec_value);
        break;
        case ARRAY_ACS:
            initval = res.len / 10;
            int     asc_value = 10;          // for now!!! It'll be changed
            for (int i = 0; i < res.len; i++)
                res.v[i] = initval += rndint(asc_value);
        break;
        case ARRAY_RND:
        case ARRAY_ZERO:
            for (int i = 0; i < res.len; i++){ // iter??? TODO:
                switch (typ){
                    case ARRAY_RND:
                        res.v[i] = rndint(10 * res.len);
                    break;
                    case ARRAY_ZERO:
                        res.v[i] = 0;
                    break;
                    default:
                    break;
                }
            }
        break;
        case ARRAY_NONE:
            // just do nothing
        break;
        default:
            logmsg("Unsupported type %d", typ);
        break;
    }
    return logret(res, "sz = %d", res.sz);
}

void                    IArray_free(IArray *val){
    if (val && val->v){
        free(val->v);
        val->v = 0;
    }
}

int                     IArray_fprint(FILE *f, IArray val, int limit){
    int     array_rec_line = 20;        // TODO: should be external to be able to replace
    int     cnt = 0, i;
    limit = (limit == 0)? val.len : (limit < val.len) ? limit : val.len;

    for (i = 0; i < limit; i++){
        cnt += fprintf(f, "%d\t", val.v[i]);
        if ( (i + 1) % array_rec_line == 0)
            cnt += fprintf(f, "\n");
    }
    if (i < val.len)
        cnt += fprintf(f, "and more (%d) ...\n", val.len - i);
    return cnt;
}

static inline int       IArray_print(IArray val, int limit){
    return IArray_fprint(stdout, val, limit);
}

// internal proc
static int              binsearch(int x, IArray arr);

static int              binsearch_typ2(int x, IArray arr);

const char *usage_str = "Usage: %s <len> <type of gen:r (random)\n";

int                     main(int argc, const char *argv[]){
    static const char *logfilename = "log/binsearch.3.1.log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir") 

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s binsearch KR task 3.1\n", __FILE__); 
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (!check_arg(3, usage_str, *argv) ){
        return 1;
    }
    int     arr_len = atoi(argv[1]);
    char    gen_type = toupper(*argv[2]);

    if (!inv (arr_len > 0 && gen_type == 'A' || gen_type == 'D', "only acsending (A) or Descending (D) generator are supported") )
        return 2;

    if (gen_type == 'A')
        gen_type = ARRAY_ACS;
    else
        gen_type = ARRAY_DESC;

    IArray arr1 = IArray_create(arr_len, gen_type);

    IArray_print(arr1, 0);

    logclose("...");
    return 0;
}

// from KR p71
static int              binsearch(int x, IArray arr){
    int l, r, m;
    // TODO:
    return m;
}


