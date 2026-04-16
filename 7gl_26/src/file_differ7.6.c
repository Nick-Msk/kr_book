#include <stdio.h>
#include <string.h>

#include "log.h"
#include "fs.h"
#include "common.h"
#include "error.h"
#include "fileutils.h"

typedef struct Keys {
    bool        version;    // bool example
    const char       *source;
    const char       *target;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){ .version = false, .source = 0, .target = 0, __VA_ARGS__}

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
                const char *ptr;
                case 'v':
                    if (!ke->version){
                        ke->version = true;
                        params++;
                    }
                break;
                case 's':
                    // TODO: refactor that to make more pretty
                    if (argv[0][1] == '\0') {
                        if (argv[1]){
                            ptr = argv[1];
                            argv++;
                        } else
                             return userraise(-1, ERR_WRONG_PARAMETER, "-l option without value (must be integer followed), ex '-l123' or '-l 123'");
                    } else  // argv[1] + 1 is pointer to value
                        ptr = argv[0] + 1;
                    ke->source = ptr;        // save pointer
                    argv[0] += strlen(argv[0]) - 1; // shift
                    params++;
                break;
                case 't':
                    // TODO: refactor that to make more pretty
                    if (argv[0][1] == '\0') {
                        if (argv[1]){
                            ptr = argv[1];
                            argv++;
                        } else
                             return userraise(-1, ERR_WRONG_PARAMETER, "-l option without value (must be integer followed), ex '-l123' or '-l 123'");
                    } else  // argv[1] + 1 is pointer to value
                        ptr = argv[0] + 1;
                    ke->target = ptr;        // save pointer
                    argv[0] += strlen(argv[0]) - 1; // shift
                    params++;
                break;
                default:    // probaly it's possible to ignore unknows parameters
                    return userraise(-1, ERR_WRONG_PARAMETER, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d", params, argc);
}

const char *usage_str = "Usage: %s -s file1 -t file2\n";

static int              f_finddiff(FILE *restrict source, FILE *restrict target, fs *restrict s, fs *restrict t);

int                     main(int argc, const char *argv[]){
    logsimpleinit("Start");

    Keys    ke = Keysinit();
    argc = parse_keys(argv, &ke);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s: file differ 7.6\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }
    invraise(ke.source != 0 && ke.target != 0, usage_str, *argv);

    FILE    *source = fopen(ke.source, "r");
    if (!source)
        userraiseint(ERR_UNABLE_OPEN_FILE_READ, "Unable to open %s", ke.source);
    FILE    *target = fopen(ke.target, "r");
    if (!target)
        userraiseint(ERR_UNABLE_OPEN_FILE_READ, "Unable to open %s", ke.target);

    fs      source_str = FS();
    fs      target_str = FS();
    int     numdiff = f_finddiff(source, target, &source_str, &target_str);

    if (numdiff == 0)
        printf("Files equals\n");
    else if (numdiff < 0)
        fprintf(stderr, "Error occurs\n");
    else
        printf("Diff %d: %s\n%s\n", numdiff, fsstr(source_str), fsstr(target_str) );

    fsfreeall(&source_str, &target_str);
    fclose(source);
    fclose(target);
    if (!fs_alloc_check(false))
        logmsg("Warning: incorrect allocation of fs's");

    return logret(0, "end...");  // as replace of logclose()
}

// 0 means equal,
static int              f_finddiff(FILE *restrict source, FILE *restrict target, fs *restrict s, fs *restrict t){
    int     cnt = 0;
    int     src_len, trg_len, cmp;
    do {
        src_len = fgetline_fs(source, s);
        trg_len = fgetline_fs(target, t);
        if (src_len && trg_len)
            cmp = fs_cmp(s, t);
        cnt++;      // 1-st line is 1
    } while (src_len == 0 || trg_len == 0 || cmp != 0);
    if (cmp)
        return logsimpleret(cnt, "Diff on %d, [%s],[%s]", cnt, s->v, t->v);
    else
        return logsimpleret(0, "No diff");
}

