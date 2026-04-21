#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "log.h"
#include "fs.h"
#include "common.h"
#include "error.h"
#include "string_hash.h"
#include "getword.h"

typedef struct Keys {
    bool        version;    // bool example
    bool        string;
    int         size;
    char       *filename;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){ .version = false, .string = 0, .size = 15, __VA_ARGS__}

static int              parse_keys(const char *argv[], Keys *ke){
    logenter("...");
    int     argc = 1, params = 0;
    if (!ke)
        return userraiseint(ERR_WRONG_INPUT_PARAMETERS, "Zero ke!!! Error!");   // raise here
    char    c;
    while (*++argv != 0 && **argv == '-'){
        argc++;
        while ( (c = *++argv[0]) )
            switch (tolower(c)){
                case 'v':
                    if (!ke->version){
                        ke->version = true;
                        params++;
                    }
                break;
                case 'f':
                    ke->string = (char *) argv[1];        // save pointer
                    argv++;
                    argv[0] += strlen(argv[0]) - 1; // shift
                    params++;
                break;
                case 's':
                    ke->size = atoi(argv[1]);        // save value
                    argv++;
                    argv[0] += strlen(argv[0]) - 1; // shift
                    params++;
                break;
                default:    // probaly it's possible to ignore unknows parameters
                    fprintf(stderr, "Illegal option [%c]\n", c);
                    return logerr(-1, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d", params, argc);
}

const char *usage_str = "Usage: %s -v -s\n";

static int              parse_input(int size);
static int              do_test_runs(stringhash *ph, int count);

int                     main(int argc, const char *argv[]){
    logsimpleinit("Start");

    Keys    ke = Keysinit();
    argc = parse_keys(argv, &ke);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s KR, hash table, task 6.5\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    int cnt = parse_input(ke.size);
    if (cnt < 0)
        fprintf(stderr, "Procedding failed\n");
    else
        printf("Processed %d\n", cnt);

    if (!fs_alloc_check(false))
        logmsg("Warning: incorrect allocation of fs's");

    return logret(0, "end...");  // as replace of logclose()
}


static int              process_define      (stringhash *h);
static int              process_undef       (stringhash *h);
static int              process_test        (stringhash *h);
static int              process_clear       (stringhash *h);
static int              process_printall    (stringhash *h);
static int              process_print       (stringhash *h);
static int              process_count       (stringhash *h);

static int              parse_input(int size){

    stringhash      h = strhash_create(size, HASH_SIMPLE);    // 200 not sure

    // do some tests here
    int     cnt = 0;
    Lexem   lex = lexeminit();
    printf("Start with (%d)>", h.sz);
    // TODO: try to catch ctrl+C here!
    while (getlexem(&lex, false) ){
        if (lex.typ == LEXEM_CMD){
            // TODO: refactor that to normat search command API
            if (lexem_eq(&lex, "define", 3) )
                process_define(&h);
            else if (lexem_eq(&lex, "undef", 3) )
                process_undef(&h);
            else if (lexem_eq(&lex, "test", 3) )
                process_test(&h);
            else if (lexem_eq(&lex, "clear", 3) )
                process_clear(&h);
            else if (lexem_eq(&lex, "count", 3) )
                process_count(&h);
            else if (lexem_eq(&lex, "printall", 3) )
                process_printall(&h);
            else if (lexem_eq(&lex, "p", 1) )
                process_print(&h);
            else if (lexem_eq(&lex, "quit", 1) ){
                printf("Done\n");
                break;
            }
            else
                printf("Unknown command [%s]", lexemstr(lex));
        } else
            fprintf(stderr, "Incorrent lexem type %d:%s", lex.typ, Lexemtype_str(lex.typ) );
        printf("\n(%d)>", h.sz);
    }
    strhashfree(h);
    lexemfree(lex);
    return cnt;
}

static int              process_define(stringhash * ph){

    static int cnt = 0;

    int     ret = 0;

    Lexem   lex = lexeminit(), *plex = &lex;
    if (!getlexem(plex, false) )  // EOF?
        return logsimpleactret(lexemfree(lex), -1, "Unable to parse next lexem (name)");
    fs name = FS();
    fscpy(name, plex->str);
    if (!getlexem(plex, false) )  // EOF?
        return logsimpleactret((lexemfree(lex), fsfree(name) ), -1, "Unable to parse next lexem (value)");
    if (plex->typ == LEXEM_WORD || plex->typ == LEXEM_INT || plex->typ == LEXEM_FLOAT){
        if (!strhash_fsinstall(ph, &name, &plex->str) )
            fprintf(stderr, "Unable to install [%s:%s]\n", fsstr(name), fsstr(plex->str) );
        else
            printf("Installed! (%d)\n", ++cnt), ret = 1;
    } else
        fprintf(stderr, "Incorrent lexem type %d:%s", plex->typ, lexem_str(plex) );
    fsfree(name);
    lexemfree(lex);
    return logsimpleret(ret, "Installed %d %d", ret, cnt);
}

static int              process_undef(stringhash * ph){

    static int cnt = 0;

    int ret = 0;
    Lexem   lex = lexeminit(), *plex = &lex;
    if (!getlexem(plex, false) )  // EOF?
        return logsimpleactret(lexemfree(lex), -1, "Unable to parse next lexem (name)");
    if (plex->typ == LEXEM_WORD){
        if (strhash_undef(ph, lexem_str(plex) ) )
            printf("Removed [%s]\n", lexem_str(plex) ), ret = 1;
        else
            printf("Not found [%s]\n", lexem_str(plex) );
    } else
        fprintf(stderr, "Incorrent lexem type %d:%s", plex->typ, lexem_str(plex) );
    lexemfree(lex);
    return logsimpleret(ret, "Removed current %d, total %d", ret, ++cnt);
}

static int              process_clear(stringhash *ph){
    strhash_clear(ph);
    return 1;
}

static int              process_printall(stringhash *ph){
    return strhash_printall(ph);
}

static int              process_print(stringhash *ph){

    Lexem   lex = lexeminit(), *plex = &lex;
    if (!getlexem(plex, false) )  // EOF?
        return logsimpleactret(lexemfree(lex), -1, "Unable to parse next lexem (name)");
    if (plex->typ == LEXEM_WORD){
        strhash_print(ph, lexem_str(plex) );
    } else
        fprintf(stderr, "Incorrent lexem type %d:%s", plex->typ, lexem_str(plex) );
    lexemfree(lex);
    return 1;
}

static int              process_count(stringhash * ph){
    printf("%d\n", strhash_cnt(ph) );
    return 1;
}

static int              process_test(stringhash *ph){

    int ret = 0;
    Lexem   lex = lexeminit(), *plex = &lex;
    if (!getlexem(plex, false) )  // EOF?
        return logsimpleactret(lexemfree(lex), -1, "Unable to parse next lexem (cpunt of test)");
    if (plex->typ == LEXEM_INT){
        int cnt = atoi(lexem_str(plex) );
        ret = do_test_runs(ph, cnt);
    } else
        fprintf(stderr, "Incorrent lexem type %d:%s", plex->typ, lexem_str(plex) );
    lexemfree(lex);
    return ret;
}

static int              do_test_runs(stringhash *ph, int count){

    fs      name = FS(),
            value = FS();
    int     rem_cnt = 0;
    srand(time(NULL));

    for (int i = 0; i < count; i++){
        fssprintf(name, "Name_%d", i);
        fssprintf(value, "%d", i + 1);
        if (!strhash_fsinstall(ph, &name, &value) )
            return logsimpleactret( (fsfree(name), fsfree(value) ), -1, "Failed on %d", i);
    }
    strhash_printall(ph);
    for (int i = 0; i < count; i += rndint(3) + 1 ){
        fssprintf(name, "Name_%d", i);
        strhash_undef(ph, name.v);
        rem_cnt++;
    }
    printf("Removed %d, remained %d\n", rem_cnt, strhash_cnt(ph) );
    strhash_printall(ph);
    fsfree(name), fsfree(value);
    return count;
}

