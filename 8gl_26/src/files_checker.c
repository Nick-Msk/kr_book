#include <unistd.h>

#include "log.h"
#include "common.h"
#include "bool.h"
#include "error.h"
#include "getword.h"
#include "files.p8.5.h"
#include "fs.h"
#include "proc_actions.h"
#include "command_executor.h"

const char                 *usage_str = "Usage: %s -v -f\n";

typedef struct Keys {
    bool        version;    // bool example
    const char *filename;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){ .version = false, .filename = 0, __VA_ARGS__}

static int              parse_keys(const char *argv[], Keys *ke){
    logenter("...");
    int         argc = 1, params = 0;

    if (!ke)
        return userraiseint(ERR_WRONG_INPUT_PARAMETERS, "Zero ke!!! Error!");   // raise here
    char    c;
    while (*++argv != 0 && **argv == '-'){
        argc++;
        const char *ptr;
        while ( (c = *++argv[0]) )
            switch (tolower(c)){
                case 'v':
                    ke->version = true;
                    params++;
                break;
                case 'f':
                    // TODO: refactor that to make more pretty
                    if (argv[0][1] == '\0') {
                        if (argv[1]){
                            ptr = argv[1];
                            argv++;
                        } else
                             return userraise(-1, ERR_WRONG_PARAMETER, "-l option without value (must be integer followed), ex '-l123' or '-l 123'");
                    } else  // argv[1] + 1 is pointer to value
                        ptr = argv[0] + 1;
                    ke->filename = ptr;        // save pointer
                    argv[0] += strlen(argv[0]) - 1; // shift
                    params++;
                break;
                default:    // probaly it's possible to ignore unknows parameters
                    return userraise(-1, ERR_WRONG_PARAMETER,  "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d", params, argc);
}

static bool         cyclerun(const char *filename);

int     main(int argc, const char *argv[]){
    logsimpleinit("Start");

    Keys    ke = Keysinit();
    argc = parse_keys(argv, &ke);

    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version){
        printf("%s files checker, 8.2-8.4\n", __FILE__);
        printf(usage_str, *argv);
        return 0;
    }

    int     res = 0;
    if (!cyclerun(ke.filename) )
        printf("Failed...\n"), res = 3;
    else
        printf("Finished!\n");

    if (!fs_alloc_check(false))
        logmsg("Warning: incorrect allocation of fs's");

    return logret(res, "end...");  // as replace of logclose()
}

// filled locally!
static Command cmds[] = {
    CommandInit(.name = "quit"          , .proc = proc_quit         , .desc = "Just quit the runner"                                        )
  , CommandInit(.name = "help"          , .proc = proc_help         , .desc = "Type all command and descriptions"                           )
  , CommandInit(.name = "techprint"     , .proc = proc_techprint    , .desc = "Technical print of context"                                  )
    // -- MFILE procs
  , CommandInit(.name = "create"        , .proc = proc_create       , .desc = "create <filaname>, create filename for write"                )
  , CommandInit(.name = "close"         , .proc = proc_close        , .desc = "close <read/write>, close read or write assiciated file"     )
  , CommandInit(.name = "open"          , .proc = proc_open         , .desc = "open <filaname>, open filename for read"                     )
  , CommandInit(.name = "eof"           , .proc = proc_eof          , .desc = "Check if READ file in EOF"                                   )
  , CommandInit(.name = "getpos"        , .proc = proc_getpos       , .desc = "getpos <read/write> - show current position"                 )
  , CommandInit(.name = "fileno"        , .proc = proc_fileno       , .desc = "fileno <read/write> get file no"                             )
  , CommandInit(.name = "!!!END!!!"     , .proc = 0                 , .desc = 0                                                             )
};

// move out??? not sure
static bool         cyclerun(const char *filename){
    logenter("%s", filename);

    bool        ret = true;

    Lexem       lex = lexeminit();
    Context     ctx = ContextInit(.lex = &lex, .cmds = cmds);   // .cmds - all commands
    printf("Start with (%s)>", filename);

    while (!ctx.quit && getlexem(&lex, false) ){
        if (lex.typ == LEXEM_CMD){
            // find + exec
            process_command( fsstr(lex.str), cmds, &ctx);
        } else
            fprintf(stderr, "Incorrent lexem type %d:%s\n", lex.typ, Lexemtype_str(lex.typ) );
        printf("Cmd>");
    }
    lexemfree(lex);
    return logret(ret, "%s", bool_str(ret) );
}

