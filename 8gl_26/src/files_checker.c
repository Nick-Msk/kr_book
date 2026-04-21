#include <unistd.h>

#include "log.h"
#include "common.h"
#include "bool.h"
#include "error.h"
#include "getword.h"
#include "files.p8.5.h"
#include "fs.h"

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

static bool         start_checking(const char *filename);

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
    if (!start_checking(ke.filename) )
        printf("Failed...\n"), res = 3;
    else
        printf("Finished!\n");

    return logret(res, "end...");  // as replace of logclose()
}

typedef struct {
    Lexem       *lex;
    MFILE       *mfr;   // read
    MFILE       *mfw;   // write
    bool        quit;   // flag for quit
    // ....
} Context;
#define             ContextInit(...) (Context) {.lex = 0, .mfr = 0, .mfw = 0, .quit = false,  ##__VA_ARGS__}

typedef int         (*process_unit)(Context *c);   // function, process context

typedef struct Command {
    const char *    name;
    process_unit    proc;
} Command;

#define             CommandInit(...) (Command) {.name = 0, .proc = 0, ##__VA_ARGS__}

// example of working procedures
int                 proc_quit(Context *ctx){
    ctx->quit = true;
    return 1;
}

int                 proc_create(Context *ctx){
    logsimple("ctx %p, lex %p", ctx, ctx ? ctx->lex : 0);
    Lexem *l = ctx->lex;
    if (getlexem(l, false) )
        if (l->typ == LEXEM_WORD){
            const char *fname = fsstr(l->str);
            MFILE      *mf;
            if ( (mf = mopen(fname, "w") ) == 0)
                fprintf(stderr, "Unable to create %s\n", fname);
            else {
                printf("File %s is opened for write\n", fname);
                ctx->mfw = mf;       // setup ctx;
                return logsimpleret(1, "File %s was created and opened for write", fname);
            }
        } else
            fprintf(stderr, "Word expected but (%s)", Lexemtype_str(ctx->lex->typ) );
    else
        fprintf(stderr, "Out of input\n");
    return logsimpleerr(-1, "Failed to create");  // failed
}

static Command cmds[] = {
    CommandInit(.name = "quit"      , .proc = proc_quit )
  , CommandInit(.name = "create"    , .proc = proc_create )
  , CommandInit(.name = "!!!END!!!" , .proc = 0 )
};

// WILL BE MOVED Out TO command_executor, so can't usr cmds directly
// go through cmd until cmd->proc is null
static bool         process_command(const char *restrict name, Command *restrict cmd, Context *restrict ctx){
    logenter("Cmd [%s]", name);
    // 1-st version, just a fullscan
    while (cmd->proc){
        logauto(cmd->name);
        if (strcmp(name, cmd->name ) == 0){
            cmd->proc(ctx);
            return logret(true, "Command %s was processed", name);
        }
        cmd++;      // next
    }
    return logerr(false, "Command %s not found", name);
}

static bool         start_checking(const char *filename){
    logenter("%s", filename);

    bool        ret = true;

    Lexem       lex = lexeminit();
    Context     ctx = ContextInit(.lex = &lex);
    printf("Start with (%s)>", filename);

    while (!ctx.quit && getlexem(&lex, false) ){
        if (lex.typ == LEXEM_CMD){
            // find + exec
            process_command( fsstr(lex.str), cmds, &ctx);
        } else
            fprintf(stderr, "Incorrent lexem type %d:%s", lex.typ, Lexemtype_str(lex.typ) );
    }
    return logret(ret, "%s", bool_str(ret) );
}

