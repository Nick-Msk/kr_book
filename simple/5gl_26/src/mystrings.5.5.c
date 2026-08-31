#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "log.h"
#include "common.h"
#include "checker.h"

static const char              *mystrncat(char *restrict t, const char *restrict s, int n);
static const char              *mystrncpy(char *restrict t, const char *restrict s, int n);
static int                      mystrncmp(const char *restrict t, const char *restrict s, int n);
static const char              *processcmd(const char *cmd, const char *restrict par1, const char *restrict par2, int n);

const char *usage_str = "Usage: %s <method:COPY/CAT/CMP> <to:str> <from:str> <n:int>\n";

int                             main(int argc, const char *argv[]){
    static const char *logfilename = "log/"__FILE__".log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    if (argc > 1){
        if (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0){
            printf("%s KR mystrings, task 5.5\n", __FILE__);
            printf(usage_str, *argv);
            return 0;
        }
    }
    if (!check_arg(5, usage_str, *argv) ){
        return 2;
    }

    // TODO: rework with fs!
    // example fs:
    // const char *str = fsstr(s); // no copy here!
    // const char *str = fsstrcopy(s);  // copy amd malloc
    // fs s = fsinit(100); // empty
    // fs s = fsempty();    // empty with 0 alloc
    // fs s = fscopy(argv[1]); // copy origin!! and malloc here
    // fs ts = fsclone(s);
    // fsshrink(s) // srink to length!
    // fsresize(s, 1000);   // resize to value
    // fs s = fsliteral("abc"); // read only, no auto-alloc, use fsclone() to create normal fs
    // char c = get(s)[5]; or  char c = s.v[5]; // no allocation
    // char c = elem(s, 5) // with auto allocation
    // elem(s, 6) = 'c'; // with autoalloc
    // get(s, 14) = 'r' or get(s[14]) = 'x';    // no allocation, just s.v[14] = 'x';

    int         n = atoi(argv[4]);
    const char *cmd = argv[1];

    const char *s = processcmd(cmd, argv[2], argv[3], n);
    printf("Returned from %s: [%s]\n", cmd, s);
    free( (void *) s);
    logclose("...");
    return 0;
}

static const char                     *processcopy(const char *restrict par1, const char *restrict par2, int n){
    // par1 is ignored
    char    *s = malloc(n + 1);
        if (!inv(s != 0, "Unable to allocated %d", n + 1) )
            return 0;
    // exec
    return mystrncpy(s, par2, n);
}

static const char                     *processcmp(const char *restrict par1, const char *restrict par2, int n){
    char *s = malloc(200);
    const char *res = "Equal";
    if (mystrncmp(par1, par2, n) != 0)
        res = "Not equal";

    snprintf(s, 200, "%s (%d)", res, n);
    return s;
}

static const char                     *processcat(const char *restrict par1, const char *restrict par2, int n){
    int     len = strlen(par1);
    int     sz  = n + len + 1;
    char    *s  = malloc(n);
    if (!inv(s != 0, "Unable to allocated %d", sz) )
         return 0;
    memcpy(s, par1, len + 1);   // init
    return mystrncat(s, par2, n);
}

// just wrapper to launch
static const char                      *processcmd(const char *cmd, const char *restrict par1, const char *restrict par2, int n){
    const char    *ret = 0;

    if (strcasecmp(cmd, "COPY") == 0)
        ret = processcopy(par1, par2, n);
    else if (strcasecmp(cmd, "CAT") == 0)
        ret = processcat(par1, par2, n);
    else if (strcasecmp(cmd, "CMP") == 0)
        ret = processcmp(par1, par2, n);

    return ret;
}

static const char              *mystrncat(char * restrict t, const char * restrict s, int n){
    char    *res = t;
    while (*t != '\0')
        t++;
    while (n-- > 0 && (*t++ = *s++) )
        ;
    return res;
}

static const char              *mystrncpy(char * restrict t, const char * restrict s, int n){
    const char *res = t;
    while (n-- > 0 && (*t++ = *s++) )
        ;
    if (*s != '\0')
        *t = '\0';  // finallize
    return res;
}

static int                      mystrncmp(const char * restrict t, const char * restrict s, int n){
    while (n > 0 && *t == *s)
        t++, s++, n--;
    logsimple("n %d, *t [%c] *s [%c]", n, *t, *s);
    if (n == 0 || (*t == '\0' && *s == '\0') )
        return 0;
    else
        return *s - *t;
}


