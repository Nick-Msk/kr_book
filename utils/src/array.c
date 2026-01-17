

#include "array.h"

/********************************************************************
                 <SKELETON> MODULE IMPLEMENTATION
********************************************************************/

// static globals

// internal type

// ---------- pseudo-header for utility procedures -----------------

// ------------------------------ Utilities ------------------------

// -------------------------- (Utility) printers -------------------

// --------------------------- API ---------------------------------

#define                 IArray_init(...) (IArray){.len = 0, .sz = 0, .v = 0}

// CREATE  and fill with method
// increase and shrink are reuiqred too
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

// -------------------------- (API) printers -----------------------

int                     IArray_fprint(FILE *f, IArray val, int limit){
    int     array_rec_line = 20;        // TODO: should be external to be able to replace
    int     cnt = 0, i;
    limit = (limit == 0)? val.len : (limit < val.len) ? limit : val.len;

    for (i = 0; i < limit; i++){
        cnt += fprintf(f, "%6d\t", val.v[i]);
        if ( (i + 1) % array_rec_line == 0)
            cnt += fprintf(f, "\n");
    }
    if (i < val.len)
        cnt += fprintf(f, "and more (%d) ...\n", val.len - i);
    return cnt;
}


// -------------------------------Testing --------------------------

#ifdef ARRAYTESTING

#include "testing.h"
#include <signal.h>

// ------------------------- TEST 1 ---------------------------------

// ------------------------- TEST 2 ---------------------------------

// -------------------------------------------------------------------
int
main(int argc, char *argv[])
{
    const char *logfilename = "checker.log";

    if (argc > 1)
        logfilename = argv[1];

    loginit(logfilename, false, 0, "Starting");

    testenginestd(
        testnew(.f2 = tf1, .num = 1, .name = "Simple invariant text"       , .desc = "", .mandatory=true)
      , testnew(.f2 = tf2, .num = 2, .name = "Complex invariant test"      , .desc = "", .mandatory=true)
      //, testnew(.f2 = f3, .num = 3, .name = "Interrupt raising test"        , .desc = "Exception test."                                                             , .mandatory=true)
      //, testnew(.f2 = f4, .num = 4, .name = "System error test."            , .desc = "System error raising test (w/o exception)."  , .mandatory=true)
    );

        logclose("end...");
    return 0;
}


#endif /* ARRAYTESTING */

