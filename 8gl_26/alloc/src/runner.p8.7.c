#include <stdio.h>
#include <string.h>

#include "log.h"
#include "common.h"
#include "error.h"
#include "checker.h"
#include "parse_keys.h"
#include "fs.h"
#include "alloc.h"
#include "array.h"
#include "guard.h"

typedef struct Keys {
    bool              version;    // bool example
    //int               initsize;
    bool              abfreekey;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){ .version = false, .abfreekey = false, __VA_ARGS__}
static int              parse_keys(const char *argv[], Keys *ke){

    invraise(ke, "Zero ke!!! Error!"); // raise here
    logenter("...");

    int     argc = 1, params = 0;
    char    c;
    const char *ptr;

    while (*++argv != 0 && **argv == '-'){
        argc++;
        while ( (c = *++argv[0]) )
            switch (tolower(c)){
                parse_bool('v', version);
                parse_bool('a', abfreekey);
                //parse_int('i', initsize);
                default:    // probaly it's possible to ignore unknows parameters
                    return userraise(-1, ERR_WRONG_PARAMETER, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d, %s", params, argc, ptr = 0);
}

static bool             test1(unsigned);
static bool             test2(void);
static bool             test3(unsigned);
static bool             test4(void);
static bool             test5(void);

const char *usage_str = "Usage: %s -a\n";

int                     main(int argc, const char *argv[]){
    logsimpleinit("Start");

    Keys    ke = Keysinit();
    argc = parse_keys(argv, &ke);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s KR test for alloc p8.6, pg198 \n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    if (ke.abfreekey)
        printf("\nTest5: %s\n", bool_str(test5() ) );
    else {
        unsigned sz = 1000;
        printf("\nTest1: %s\n", bool_str(test1(sz) ) );

        printf("\nTest2: %s\n", bool_str(test2() ) );

        printf("\nTest3: %s\n", bool_str(test3(500) ) );
        bool res = test4();
        logmsg("res = %d\n", res);
        printf("\nTest4: %s\n", bool_str(res) );
    }

    if (!fs_alloc_check(false))
        logmsg("Warning: incorrect allocation of fs's");

    return logret(0, "end...");  // as replace of logclose()
}

static bool             fill_and_check(char *t, unsigned sz){
    char c = itoc(sz % 10);
    while (--sz)
        t[sz] = c;
    return true;
}

static bool             test1(unsigned sz){
    char    *t = alloc(sz);
    afprint(stdout, "\nafter 1 %u\n", sz);
    char    *t1 = alloc(sz * 4);
    afprint(stdout, "\nafter 2 %u\n", sz * 4);
    char    *t2 = alloc(sz * 6);
    afprint(stdout, "\nafter 3 %u\n", sz * 6);

    int  lsz = 100;
    long *l = alloct(lsz, sizeof(long) );
    afprint(stdout, "\nafter 4\n");

    for (int i = 0; i < lsz; i++)
        if (l[i] != 0)
            fprintf(stderr, "l[%d] == %ld\n", i, l[i]);

    fill_and_check(t, sz);
    afprint(stdout, "\nAfter free 1\n");
    fill_and_check(t1, sz * 4);
    afprint(stdout, "\nAfter free 2\n");
    fill_and_check(t2, sz * 6);
    afprint(stdout, "\nAfter free 3\n");
    afree(t2);
    afree(t);
    afree(t1);
    return true;
}

static bool             test2(void){
    char *t = alloc(200000000);
    if (!t)
        printf("\nOk!!! Not allocated!\n");
    else
        afree(t);;
    return !t;
}

static bool             test3(unsigned initsz){
    logenter("Sz %u", initsz);
    // init pointer saver
    Array arr = PArray_create(initsz, ARRAY_ZERO);

    for (int j = 1; j < 5; j++){
        // randomly create int * objects
        for (int i = 0; i < (int) initsz; i++){
            unsigned sz = rnduint(500 - 1) + 1;
            //MODEXEC(500, logmsg("%d - %u", i, sz) );
            if (IFMOD(50) )
                logmsg("ALLOC: %d - %u", i, sz);
            if ( (arr.pv[i] = alloc(sz * sizeof(int) ) ) ==0)
                userraiseint(ERR_UNABLE_ALLOCATE, "pos %d, %u int's", i, sz);
            // fill with current value! From common
            fill_int(arr.pv[i], sz, sz);        // value the same as count
        }
        // random free now
        for (int i = 0; ; i += rndint(3) + 1){
            if (IFMOD(1) )
                logmsg("FREE: %d", i);
            if (i < (int) initsz){
                afree(arr.pv[i]);
                arr.pv[i] = 0;
            } else
                break;
        }
        printf("CYCLE %d: Remains %d of %d\n\n", j, ArrayGetcnt(arr), initsz);
        afprint(stdout, "After multiples alloc and random free...");
    }

    printf("I: Remains %d of %d\n\n", ArrayGetcnt(arr), initsz);
    // free remains
    for (int i = 0; i < (int) initsz; i++){
        if (arr.pv[i]){
            afree(arr.pv[i]);
            arr.pv[i] = 0;
        }
    }
    printf("II: Remains %d of %d\n\n", ArrayGetcnt(arr), initsz);
    afprint(stdout, "After all free");

    Arrayfree(arr);

    return logret(true, "Done");
}

static bool             test4(void){
    int   sz1 = 100, sz2 = 200, sz3 = 1, sz4 = 1000;
    logenter("start alloc_type\n");
    double  *d = alloc_type(sz1, double);
    long    *l = alloc_type(sz2, long);
    int     *i = alloc_type(sz3, int);
    double  *f = alloc_type(sz4, double);
    // check that filler with zero
    for (int i = 0; i < sz1; i++)
        if (d[i] != 0.0){
            printf("%d - %lf\n", i, d[i]);
            return logret(false, "%d - %lf\n", i, d[i]);
        }
    for (int i = 0; i < sz2; i++)
        if (l[i] != 0L){
            printf("%d - %ld\n", i, l[i]);
            return logret(false, "%d - %ld\n", i, l[i]);
        }
    for (int j = 0; j < sz3; j++)
        if (i[j] != 0){
            printf("%d - %d\n", j, i[j]);
            return logret(false, "%d - %d\n", j, i[j]);
        }
    for (int j = 0; j < sz4; j++)
        if (f[j] != 0.0f){
            printf("%d - %f\n", j, f[j]);
            return logret(false, "%d - %f\n", j, f[j]);
        }
    // free
    printf("Free...\n");
    afree(f);
    afree(d);
    afree(i);
    afree(l);
    return logret(true, "Ok");
}

#define                 ARR_SZ 1024 * 1024 * 20

static bool             test5(void){
    static char bigarray[ARR_SZ];

    abfree(bigarray, ARR_SZ);
    int    *a = alloc_type(1024 * 1024, int);
    double *d = alloc_type(1024 * 1024, double);

    afprint(stdout, "\nafter 2\n");

    return logsimpleret(true, "Ok");
}


static bool             test6(void){
    // only  burn test for now
    return test3(5000);
}
