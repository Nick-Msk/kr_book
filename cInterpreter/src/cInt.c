#include "fs.h"
#include "log.h"
#include "checker.h"
#include "guard.h"
#include "error.h"
#include "command_executor.h"
#include "parse_keys.h"
#include "core_impl.h"
#include "getword.h"

typedef struct Keys {
    bool              version;    // bool example
    bool              unbuf;
    const char       *infilename;
    const char       *outfilename;
    // ...
} Keys;

#define                 Keysinit(...) (Keys){ .version = false, .unbuf = false, .infilename = 0, .outfilename = 0, __VA_ARGS__}

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
                parse_bool('u', unbuf);
                parse_string('f', infilename);
                parse_string('o', outfilename);
                default:    // probaly it's possible to ignore unknows parameters
                    return userraise(-1, ERR_WRONG_PARAMETER, "Illegal [%c], params [%d] argc %d", c, params, argc);
            }
    }
    return logret(argc, "params %d, argc %d, %s", params, argc, ptr = 0);
}

const char *usage_str = "Usage: %s ... TODO\n";
const char *version = "0.1 (init)";

static bool             launch(const Keys *s);

int                     main(int argc, const char *argv[]){
    logsimpleinit("Start");

    Keys    ke = Keysinit();
    argc = parse_keys(argv, &ke);


    if (argc < 0) {
        printf(usage_str, *argv);
        return 1;
    }
    if (ke.version || ke.outfilename == 0 || ke.infilename == 0){
        printf("%s, cInterprepet ver %s\n", __FILE__, version);
        printf(usage_str, *argv);
        return 0;
    }

    launch(&ke);

    return logret(0, "end...");  // as replace of logclose()
}

// filled locally!
static Command cmds[] = {
    CommandInit(.name = "quit"          , .shortlen = 1, .proc = proc_quit         , .desc = "Just quit the runner"                                            )
  , CommandInit(.name = "help"          , .shortlen = 1, .proc = proc_help         , .desc = "Type all command and descriptions"                               )
  , CommandInit(.name = "techprint"     , .shortlen = 4, .proc = proc_techprint    , .desc = "Technical print of run-time data"                                )
    // -------------------------------------------
};

static bool             launch(const Keys *s){
    logenter("...");

    bool        ret = true;
    Lexem       lex = lexeminit();
    Runtimedata ctx = RuntimedataInit(.lex = &lex, .cmds = cmds);   // .cmds - all commands

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

