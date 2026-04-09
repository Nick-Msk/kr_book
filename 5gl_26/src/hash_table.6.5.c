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

static int              proc_defines(int size);
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

    int cnt = proc_defines(ke.size);
    if (cnt < 0)
        fprintf(stderr, "Procedding failed\n");
    else
        printf("Processed %d\n", cnt);

    if (!fs_alloc_check(false))
        logmsg("Warning: incorrect allocation of fs's");

    return logret(0, "end...");  // as replace of logclose()
}

static int              proc_defines(int size){

    stringhash      h = strhash_create(size, HASH_SIMPLE);    // 200 not sure

    // do some tests here
    int     cnt = 0;
    fs      name = FS();
    Lexem   lex = Lexeminit();
    bool    def_flag = false, undef_flag = false, print_flag = false, test_flag = false;;

    printf("Start with (%d)>", h.sz);
    while (getlexem(&lex, false) ){
        // TODO: refactor to normal, without flags
        if (def_flag){
            def_flag = false;
            fscpy(name, lex.str);
            if (!getlexem(&lex, false) )  // EOF?
                continue;
            if (lex.typ == LEXEM_WORD || lex.typ == LEXEM_INT || lex.typ == LEXEM_FLOAT){
                if (!strhash_install(&h, fsstr(name), fsstr(lex.str) ) )
                    fprintf(stderr, "Unable to install [%s:%s]\n", fsstr(name), fsstr(lex.str) );
                else
                    printf("Installed! (%d)\n", ++cnt);
            } else
                fprintf(stderr, "Incorrent lexem type %d:%s", lex.typ, Lexemstr(lex) );
        }
        else if (undef_flag){
            undef_flag = false;
            if (lex.typ == LEXEM_WORD){
                if (strhash_undef(&h, Lexemstr(lex) ) )
                    printf("Removed [%s]\n", Lexemstr(lex) );
                else
                    printf("Not found [%s]\n", Lexemstr(lex) );
            } else
                fprintf(stderr, "Incorrent lexem type %d:%s", lex.typ, Lexemstr(lex) );
        }  else if (test_flag){
            test_flag = false;
            if (lex.typ == LEXEM_INT){
                int count = atoi(Lexemstr(lex) );
                printf("Start test suite with %d\n", count);
                if (do_test_runs(&h, count) < 0)
                    fprintf(stderr, "Failed\n");
            } else
                fprintf(stderr, "Incorrent lexem type %d:%s", lex.typ, Lexemstr(lex) );
        } else if (print_flag){
            print_flag = false;
            if (lex.typ == LEXEM_WORD){
                strhash_print(&h, Lexemstr(lex) );
            } else
                fprintf(stderr, "Incorrent lexem type %d:%s", lex.typ, Lexemstr(lex) );
        } else {
            if (lex.typ == LEXEM_WORD){
                const char *name = Lexemstr(lex);
                // TODO: refactor that to normat search command API
                if (strncmp(name, "define", 3) == 0)
                    def_flag = true;
                else if (strncmp(name, "undef", 3) == 0)
                    undef_flag = true;
                else if (strncmp(name, "test", 3) == 0)
                    test_flag = true;
                else if (strncmp(name, "clear", 3) == 0)
                    strhash_clear(&h);
                else if (strncmp(name, "count", 3) == 0)
                    printf("%d\n", strhash_cnt(&h) );
                else if (strncmp(name, "printall", 3) == 0)
                    strhash_printall(&h);
                else if (strncmp(name, "p", 1) == 0)
                    print_flag = true;
                else if (strncmp(name, "quit", 1) == 0)
                    break;
                else
                    printf("Unknown command [%s]", name);
            } else
                fprintf(stderr, "Incorrent lexem type %d:%s", lex.typ, Lexemstr(lex) );
        }
        printf("\n>");
    }
    strhashfree(h);
    fsfree(name);
    Lexemfree(lex);
    return cnt;
}

static int              do_test_runs(stringhash *ph, int count){
    logauto(count);
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

