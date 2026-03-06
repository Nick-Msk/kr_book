#include <stdio.h>
#include <strings.h>
#include <ctype.h>

#include "log.h"
#include "checker.h"
#include "common.h"
#include "fileutils.h"
#include "fs.h"
#include "error.h"

typedef struct Keys {
    bool    version;
    bool    sens;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){ .version = false, .sens = true, __VA_ARGS__}

static int              parse_keys(const char *argv[], Keys *ke){
    logenter("...");
    int     argc = 1, params = 0;
    if (!ke)
        return userraiseint(-1, "Zero ke!!! Error!");   // raise here
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
                case 'i':
                    if (!ke->sens){
                        ke->sens = false;
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

static const char   *usage_str = "Usage: %s -v -i\n";

typedef struct {
    const char      *word;
    int              count;
} tkeys;

static tkeys             keytab[] = {
    {"auto"      , 0},
    {"break"     , 0},
    {"case"      , 0},
    {"char"      , 0},
    {"const"     , 0},
    {"continue"  , 0},
    {"default"   , 0},
    {"do"        , 0},
    {"double"    , 0},
    {"else"      , 0},
    {"enum"      , 0},
    {"extern"    , 0},
    {"float"     , 0},
    {"for"       , 0},
    {"goto"      , 0},
    {"if"        , 0},
    {"int"       , 0},
    {"long"      , 0},
    {"register"  , 0},
    {"return"    , 0},
    {"short"     , 0},
    {"signed"    , 0},
    {"sizeof"    , 0},
    {"static"    , 0},
    {"struct"    , 0},
    {"switch"    , 0},
    {"typedef"   , 0},
    {"union"     , 0},
    {"unsigned"  , 0},
    {"void"      , 0},
    {"volatile"  , 0},
    {"while"     , 0},
    {0x0         , 0}
};

static int               tkeys_print(const tkeys *arr);

static int               tkey_binsearch(fs word, tkeys *tab, int n);

// probably it's better from fileutils
static fs                getword(FILE *f, fs str);

int                      main(int argc, const char *argv[]){
    logsimpleinit("Start");

    Keys    ke = Keysinit();
    argc = parse_keys(argv, &ke);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s KR find_keyword p6.3\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    FILE   *f = stdin;  // -f?
    int     n;
    fs      s = FS();    // init with alloc
    while (fsisempty(s = getword(f, s) ) ) {
        if (isalpha(s.v[0] ) )
            if ( (n = tkey_binsearch(s, keytab, COUNT(keytab) - 1) ) >= 0)
                keytab[n].count++;
    }
    fsfree(s);
    printf("Total: %d\n", tkeys_print(keytab) );

    return logret(0, "end...");  // as replace of logclose()
}

static int               tkey_binsearch(fs word, tkeys *tab, int n){
    int cond, low = 0, high = n - 1,  mid;
    const char *s = fsstr(word);
    while (low <= high){
        mid = (low + high) / 2;
        if ( (cond = strcmp(s, tab[mid].word ) ) < 0)
            high = mid - 1;
        else if (cond > 0)
            low = mid + 1;
        else
            return mid;
    }
    return -1;
}

static int               tkeys_print(const tkeys *arr){
    int     total = 0;
    while (arr->word){
        if (arr->count > 0){
            printf("%4d: %s", arr->count, arr->word);
            total += arr->count;
        }
        arr++;
    }
    return total;
}

// str must have heap alloc
static fs                getword(FILE *f, fs str){
    if (f == 0)
        f = stdin;
    
}
