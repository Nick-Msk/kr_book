#include <stdio.h>

#include "error.h"
#include "checker.h"
#include "fs.h"
#include "fs_array.h"
#include "core_impl.h"

// ------------------------------------------ Utilities -------------------------------------------

static int                      Runtimedata_techfprint(FILE *restrict out, const Runtimedata *restrict rt){
    invraise(rt != 0, "Null pointer");
    int     cnt = 0;
    if (out){
        cnt += fprintf(out, "Run time data: flname [%s]\n", rt->flname);
        if (rt->lex){
            cnt += lexem_techfprint(out, rt->lex);
        }
        cnt += fsarr_techfprint(out, &rt->body);
        cnt += fprintf(out, "END of Run time data");
    }
    return logsimpleret(cnt, "Printed %d", cnt);
}

static inline int               Runtimedata_techprint(const Runtimedata *rt){
    return Runtimedata_techfprint(stdout, rt);
}

static int                      util_save(FILE *restrict out, const fsarray *restrict arr){
    int     i;
    for (i = 0; i < fsarr_cnt(arr); i++){
        fs *s = arr->ar + i;
        fwrite(fs_str(s), 1, fs_len(s), out);
    }
    return i;
}

// --------------------------------------- AUXILLARY PROCS -------------------------------------------

// example of working procedures
// local
int                             proc_quit(Runtimedata *rt){
    rt->quit = true;
    return 1;
}

int                             proc_help(Runtimedata *rt){
    Command *cm = rt->cmds;
    while (cm->desc){
        printf("%s\t\t: %s\n", cm->name, cm->desc);
        cm++;
    }
    return 1;
}

int                             proc_techdump(Runtimedata *rt){
    // just technical print
    Runtimedata_techprint(rt);
    return 1;
}

// ------------------------------------------- PROCS -------------------------------------------------

int                             proc_load(Runtimedata *rt){
    invraise(rt != 0, "Null pointer");
    int     cnt = 0;
    if (rt->fl){
        // TODO: load from file, use fileutils
    }
    return logsimpleret(cnt, "%d Lines were load", cnt);
}

int                             proc_print(Runtimedata *rt){
    invraise(rt != 0, "Null pointer");
    int     cnt = 0;
    fsarr_print(&rt->body);
    //for (int i = 0; i < fsarr_cnt(rt->body); i++){
        // TODO:         ???
    //}
    return logsimpleret(cnt, "%d Lines were load", cnt);
}

// main!
int                             proc_run(Runtimedata *rt){
    invraise(rt != 0 && rt->runfl != 0, "Null pointer");
    fprintf(rt->runfl, "#include <all.h>\nint main(int argc, const char *argv[]){\n");
    util_save(rt->runfl, &rt->body);  // apprend
    fprintf(rt->fl, "\n}\n");
    // run make cInt from here
    // not sude, via dll or directly
    // this version 0.1 will NOT switch stdout/err
    return 1;   // as OK
}

int                             proc_save(Runtimedata *rt){
    invraise(rt != 0, "Null pointer");
    int     cnt = 0;
    if (rt->fl){
        // TODO: save to file, use fileutil
        rt->fl = freopen(NULL, "w+", rt->fl);
        if (!rt->fl)
            sysraiseint("Unable to reopen %s", rt->flname);
        cnt = util_save(rt->fl, &rt->body);
    }
    return logsimpleret(cnt, "%d Lines were saved", cnt);
}

int                             proc_clear(Runtimedata *rt){
    invraise(rt != 0, "Null pointer");
    fsarr_clean(&rt->body, false);
    return logsimpleret(1, "Cleared!");
}

int                             proc_par(Runtimedata *rt){
    // TODO!!!
    return logsimpleret(1, "Pars...");
}

