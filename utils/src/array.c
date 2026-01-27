#include <limits.h>
#include "array.h"

/********************************************************************
                 <SKELETON> MODULE IMPLEMENTATION
********************************************************************/

// static globals

int                      g_array_rec_line = 20;

// internal type

// ---------- pseudo-header for utility procedures -----------------

// ------------------------------ Utilities ------------------------

// -------------------------- (Utility) printers -------------------

// --------------------------- API ---------------------------------


void                     Array_fill(Array a, ArrayFillType typ){
    int     initval;
    // fill
    switch (typ){
        case ARRAY_DESC:
            initval = 100 * a.len;   // hope it'll ne owerwelhm int
            int     dec_value = 10;          // for now!!! It'll be changed
            for (int i = 0; i < a.len; i++){
                if (Array_isint(a))
                    a.iv[i] = initval -= rndint(dec_value);
                else    // isdouble
                    a.dv[i] = initval -= rnddbl(dec_value);
            }
        break;
        case ARRAY_ACS:
            initval = a.len / 10;
            int     asc_value = 10;          // for now!!! It'll be changed
            for (int i = 0; i < a.len; i++){
                if (Array_isint(a))
                    a.iv[i] = initval += rndint(asc_value);
                else    // isdouble
                    a.dv[i] = initval += rnddbl(asc_value);
            }
        break;
        case ARRAY_RND:
        case ARRAY_ZERO:
            for (int i = 0; i < a.len; i++){ // iter??? TODO:
                switch (typ){
                    case ARRAY_RND:
                        if (Array_isint(a))
                            a.iv[i] = rndint(10 * a.len);
                        else
                            a.dv[i] = rnddbl(10 * a.len);
                    break;
                    case ARRAY_ZERO:
                        if (Array_isint(a))
                            a.iv[i] = 0;
                        else
                            a.dv[i] = 0.0;
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
            logsimple("Unsupported type %d", typ);
        break;
    }
}

// CREATE  and fill with method
// increase and shrink are reuiqred too
Array                           Array_create(int cnt, ArrayFillType filltyp, ArrayType typ){
    logenter("cnt %d, typ %s", cnt, ArrayFillTypeName(filltyp));
    Array       res = Array_init();
    res.flags      |= typ;
    res.sz          = round_up_2(cnt);   // 2^(x + 1)
    res.len         = cnt;
    int          sz = typ == ARRAY_INT ? res.sz * sizeof(int) : res.sz * sizeof(double);
    res.iv          = malloc(sz);   // iv == dv
    if (!res.iv){
        fprintf(stderr, "Unable to allocate %d bytes\n", sz);
        res.sz = INT_MIN;   // userraisesig hehe TODO:
    }
    Array_fill(res, filltyp);
    return logret(res, "sz = %d", res.sz);
}

void                    Array_free(Array *val){
    if (val && val->iv){
        free(val->iv);
        val->iv = 0;
    }
}

// -------------------------- (API) printers -----------------------

int                     Array_fprint(FILE *f, Array val, int limit){
    int     cnt = 0, i;
    limit = (limit == 0)? val.len : (limit < val.len) ? limit : val.len;

    for (i = 0; i < limit; i++){
        if (Array_isint(val) )
            cnt += fprintf(f, "%6d\t", val.iv[i]);
        else        // isdouble
            cnt += fprintf(f, "%.8g\t", val.dv[i]);
        if ( ( (i + 1) % g_array_rec_line) == 0){
            cnt += fprintf(f, "\n");
        }
    }
    if (i < val.len)
        cnt += fprintf(f, "and more (%d) ...\n", val.len - i);
    return cnt;
}

/*
void                    DArray_fill(DArray a, ArrayFillType typ){
    double     initval;
    // fill
    switch (typ){
        case ARRAY_DESC:
            initval = 100 * a.len;   // hope it'll ne owerwelhm int
            double     dec_value = 10.0;          // for now!!! It'll be changed
            for (int i = 0; i < a.len; i++)
                a.v[i] = initval -= rnddbl(dec_value);
        break;
        case ARRAY_ACS:
            initval = a.len / 10;
            double     asc_value = 10.0;          // for now!!! It'll be changed
            for (int i = 0; i < a.len; i++)
                a.v[i] = initval += rnddbl(asc_value);
        break;
        case ARRAY_RND:
        case ARRAY_ZERO:
            for (int i = 0; i < a.len; i++){ // iter??? TODO:
                switch (typ){
                    case ARRAY_RND:
                        a.v[i] = rnddbl(10 * a.len);
                    break;
                    case ARRAY_ZERO:
                        a.v[i] = 0;
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
            logsimple("Unsupported type %d", typ);
        break;
    }
}

// CREATE  and fill with method
// increase and shrink are reuiqred too
DArray                  DArray_create(int cnt, ArrayFillType typ){
    logenter("cnt %d, typ %s", cnt, ArrayFillTypeName(typ));
    DArray      res = DArray_init();

    res.sz    = round_up_2(cnt);   // 2^(x + 1)
    res.len   = cnt;
    res.v   = malloc(res.sz * sizeof(int));
    if (!res.v){
        fprintf(stderr, "Unable to allocate %d\n", res.sz);
        res.sz = INT_MIN;   // userraisesig hehe TODO:
    }
    DArray_fill(res, typ);
    return logret(res, "sz = %d", res.sz);
}

void                    DArray_free(DArray *val){
    if (val && val->v){
        free(val->v);
        val->v = 0;
    }
}

// -------------------------- (API) printers -----------------------

int                     DArray_fprint(FILE *f, DArray val, int limit){
    int     cnt = 0, i;
    limit = (limit == 0)? val.len : (limit < val.len) ? limit : val.len;

    for (i = 0; i < limit; i++){
        cnt += fprintf(f, "%.8g\t", val.v[i]);
        if ( ( (i + 1) % g_array_rec_line) == 0){
            cnt += fprintf(f, "\n");
        }
    }
    if (i < val.len)
        cnt += fprintf(f, "and more (%d) ...\n", val.len - i);
    return cnt;
}
*/

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

