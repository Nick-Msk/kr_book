#include <stdio.h>
#include <strings.h>
#include <ctype.h>

#include "log.h"
#include "checker.h"
#include "common.h"
#include "fileutils.h"
#include "fs.h"

static void             qsortfs(fs arr[], int from, int to, bool reverse, int (*comparator)(const fs *s1, const fs *s2));

static int              strcmpasdoc(const char *s1, const char *s2, bool sens);

typedef struct Keys {
    bool    numsort;
    bool    version;
    bool    reverse;
    bool    aslower;
    bool    directory_order;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){.version = false,.reverse = false, .numsort = false, .aslower = false, .directory_order = false, __VA_ARGS__}

static int              parse_keys(const char *argv[], Keys *ke){
    logenter("...");
    int     argc = 1, params = 0;
    if (!ke)
        return logerr(-1, "Zero ke!!! Error!");
    char    c;
    while (*++argv != 0 && **argv == '-'){
        logauto(*argv);
        argc++;
        while ( (c = *++argv[0]) )
            switch (tolower(c)){
                case 'v':
                    if (!ke->version){
                        ke->version = true;
                        params++;
                    }
                break;
                case 'n':
                    if (!ke->numsort){
                        ke->numsort = true;
                        params++;
                    }
                break;
                case 'r':
                    if (!ke->reverse){
                        ke->reverse = true;
                        params++;
                    }
                break;
                case 'f':
                    if (!ke->aslower){
                        ke->aslower = true;
                        params++;
                    }
                break;
                case 'd':
                    if (!ke->directory_order){
                        ke->directory_order = true;
                        params++;
                    }
                break;
                default:    // probaly it's possible to ignore unknows parameters
                    fprintf(stderr, "Illegal option [%c]\n", c);
                    return logerr(-1, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d", params, argc);
}

static int                      fsnumcmp(const fs *n1, const fs *n2){
    double d1 = atof(n1->v);
    double d2 = atof(n2->v);
    if (d1 > d2)
        return 1;
    else if (d1 < d2)
        return -1;
    else
        return 0;
}

static int                      fscmp_wrap(const fs *s1, const fs *s2){
    return fscmp(*s1, *s2);
}

static int                      fsicmp_wrap(const fs *s1, const fs *s2){
    return fsicmp(*s1, *s2);
}

// dir but senitive
static int                      fscmpdir(const fs *fs1, const fs *fs2){
    const char *s1 = fs1->v;
    const char *s2 = fs2->v;
    return strcmpasdoc(s1, s2, true);
}

// dir + insensitive
static int                      fsicmpdir(const fs *fs1, const fs *fs2){
    const char *s1 = fs1->v;
    const char *s2 = fs2->v;
    return strcmpasdoc(s1, s2, false);
}

static const char   *usage_str = "Usage: %s\n";

int                             main(int argc, const char *argv[]){
static const char *logfilename = "log/"__FILE__".log";
    loginit(logfilename, false, 0, "Start");    // TODO: rework that to LOG("logdir") or LOGAPPEND("logdir") or LOGSWITCH("logdir")

    int               ret = 0;
    int               nlines = 0;

    Keys    ke = Keysinit();
    argc = parse_keys(argv, &ke);
    fs      *lineptr;
    int (*comp)(const fs *s1, const fs *s2) = fscmp_wrap;

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s KR sortlinetask p5.11\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    if (ke.directory_order){
        if (ke.aslower)
            comp = fsicmpdir;       // dir + insensetive
        else
            comp = fscmpdir;
    } else {
        if (ke.numsort)
            comp = fsnumcmp;    // ke.aslower doesn't matter here
        else if (ke.aslower)
            comp = fsicmp_wrap;
    }


    if ((nlines = readlines(&lineptr)) > 0){
        qsortfs(lineptr, 0, nlines - 1, ke.reverse, comp);
        writelines(lineptr, nlines);
        freelines(lineptr, nlines);
    } else {
        fprintf(stderr, "Unable to read lines\n");
        ret = 1;
    }

    logclose("%d", ret);
    return ret;
}

static void          qsortfs(fs arr[], int left, int right, bool reverse, int (*comparator)(const fs *s1, const fs *s2)){
    int     last;

    int     rev = reverse ? -1 : 1;
    if (left >= right)
        return;
    fs_exch(arr + left, arr + (left + right) / 2);
    last = left;
    for (int i = left + 1; i <= right; i++)
        if (/* fscmp(arr[i], arr[left])*/ comparator(arr + i, arr + left) * rev < 0)
            fs_exch(arr + ++last, arr + i);
    fs_exch(arr + left, arr + last);
    qsortfs(arr, left, last - 1, reverse, comparator);
    qsortfs(arr, last + 1, right, reverse, comparator);
}

static inline bool      isdoc(char c){
    return c == '\0' || c == ' ' || isalnum(c);
}

static int              strcmpasdoc(const char *s1, const char *s2, bool sens){
    char    c1, c2;
    while ( (c1 = *s1) != '\0' && (c2 = *s2) != '\0' ){
        while (!isdoc(c1) )
            c1 = *++s1;
        while (!isdoc(c2) )
             c2 = *++s2;
        if (sens ? c1 == c2: tolower(c1) == tolower(c2) && c1 != '\0')
            s1++, s2++;
        else
            return sens ? c1 - c2 : tolower(c1) - tolower(c2);
    }
    return 0;
}

