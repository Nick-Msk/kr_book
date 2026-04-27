#include <stdio.h>
#include <string.h>

#include "log.h"
#include "common.h"
#include "error.h"
#include "checker.h"
#include "parse_keys.h"
#include "fs.h"
#include "alloc.h"

typedef struct Keys {
    bool              version;    // bool example
    int               initsize;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){ .version = false, .initsize = 0, __VA_ARGS__}
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
                parse_int('i', initsize);
                default:    // probaly it's possible to ignore unknows parameters
                    return userraise(-1, ERR_WRONG_PARAMETER, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d, %s", params, argc, ptr = 0);
}

static bool             test1(unsigned);
const char *usage_str = "Usage: %s -i <initsize:unsigned>\n";

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

    unsigned sz = 1000;
    if (ke.initsize > 0)
        sz = ke.initsize;
    printf("Test1: %s\n", bool_str(test1(sz) ) );

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
    afprint(stdout, "after 1 %u\n", sz);
    char    *t1 = alloc(sz * 4);
    afprint(stdout, "after 2 %u\n", sz * 4);/*
    char    *t2 = alloc(sz * 6);
    afprint(stdout, "after 3 %u\n", sz * 6);

    fill_and_check(t, sz);
    afprint(stdout, "After free 1\n");
    fill_and_check(t1, sz * 4);
    afprint(stdout, "After free 2\n");
    fill_and_check(t2, sz * 6);
    afprint(stdout, "After free 3\n");
    afree(t2);
    afree(t);
    afree(t1);*/
    return true;
}
