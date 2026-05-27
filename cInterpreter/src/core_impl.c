#include <stdio.h>

#include "error.h"
#include "checker.h"
#include "fs.h"
#include "fs_array.h"
#include "core_impl.h"
// actually only for proc_par lexem is need
#include "getword.h"

static const char           BUILD[] = "make -C res/ _run1";
static const char           RUN[] = "make -C res/ run";

// ------------------------------------------ Utilities -------------------------------------------

static int                      Runtimedata_techfprint(FILE *restrict out, const Runtimedata *restrict rt){
    invraise(rt != 0, "Null pointer");

    int     cnt = 0;
    if (out){
        cnt += fprintf(out, "Run time data: flname [%s]\n", rt->flname);
        cnt += lexem_techfprint(out, &rt->lex);
        cnt += fsarr_techfprint(out, &rt->body);
        cnt += fprintf(out, "END of Run time data");
    }
    return logsimpleret(cnt, "Printed %d", cnt);
}

static inline int               Runtimedata_techprint(const Runtimedata *rt){
    return Runtimedata_techfprint(stdout, rt);
}

static int                      util_save(FILE *restrict out, const fsarray *restrict arr, int cnt){
    int     i;
    cnt = cnt <= 0 ? fsarr_cnt(arr) : cnt;
    for (i = 0; i < cnt; i++){
        fs *s = arr->ar + i;
        fwrite(fs_str(s), 1, fs_len(s), out);
    }
    return i;
}

static bool                     util_run(const char *fname){
    if (system(RUN) != 0)
        return userraise(false, ERR_UNABLE_TO_RUN_MAKE, "Unable to run %s", RUN);
    return logsimpleret(true, "run");
}

static bool                     util_build(const char *fname){
    if (system(BUILD) != 0)
        return userraise(false, ERR_UNABLE_TO_RUN_MAKE, "Unable to %s", BUILD);
    return logsimpleret(true, "builded");
}

// check + build + run
static bool                     util_sysexec(const char *fname){
    // chech up shell
    if (!system(0) )
        return userraise(false, ERR_SHELL_NOT_AVAILABLE, "Unable to find command shell");
    // run - now directly
    if (!util_build(fname))
        return userraise(false, ERR_UNABLE_TO_RUN_MAKE, "Unble to build, check errors");
    util_run(fname);
    return logsimpleret(true, "Executed %s", fname);
}

// --------------------------------------- AUXILLARY PROCS -------------------------------------------

// example of working procedures
// local
int                             proc_quit(Runtimedata *rt){
    rt->quit = true;
    return 1;
}

int                             proc_help(Runtimedata *rt){
    invraise(rt != 0, "Null pointer");

    const Command   *cm = rt->cmds;
    logsimple("%p", cm);
    while (cm->proc){
        printf("%s(%d)\t\t: %s\n", cm->name, cm->shortlen, cm->desc);
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
        if (fsarr_floadlines(rt->fl, &rt->body, 0) < 0)
            fprintf(stderr, "Unable to load %s", rt->flname);
    }
    return logsimpleret(cnt, "%d Lines were load", cnt);
}

int                             proc_print(Runtimedata *rt){
    invraise(rt != 0, "Null pointer");

    int     cnt = 0;
    fsarr_print(&rt->body);
    return logsimpleret(cnt, "%d Lines were load", cnt);
}

// main!
int                             proc_run(Runtimedata *rt){
    invraise(rt != 0 && rt->runfl != 0, "Null pointer");

    fprintf(rt->runfl, "#include <all.h>\nint main(int argc, const char *argv[]){\n");
    util_save(rt->runfl, &rt->body, rt->bodyptr);  // apprend
    fprintf(rt->fl, "\n}\n");
    fflush(rt->fl);
    // this version 0.1 will NOT switch stdout/err
    if (!util_sysexec(rt->runflname) )
        userraise(-1, ERR_UNABLE_TO_EXEC_FILE, "Unable to exec %s", rt->runflname);
    //
    return 1;   // as OK
}

int                             proc_save(Runtimedata *rt){
    invraise(rt != 0, "Null pointer");

    int     cnt = 0;
    if (rt->fl){
        rt->fl = freopen(NULL, "w+", rt->fl);
        if (!rt->fl)
            sysraiseint("Unable to reopen %s", rt->flname);
        cnt = util_save(rt->fl, &rt->body, rt->bodyptr);
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

Runtimedata                     initRuntimedata(const char *restrict flname, const char *restrict runflname, const Command *cmds){
    invraise(flname && runflname, "Null input files names");

    Runtimedata rt = RuntimedataInit(.cmds = cmds);
    rt.fl = fopen(flname, "w+");
    if (!rt.fl)
        sysraiseint("Unable to open %s for w+", flname);
    rt.flname = flname;
    if (!rt.flname)
        sysraiseint("Unable to open %s for w+", runflname);
    rt.runflname = runflname;
    rt.body = fsarr_init(100);
    return logsimpleret(rt, "Created with %s, %s", flname, runflname);
}

void                            freeRuntimedata(Runtimedata *rt){
    invraise(rt != 0, "Null pointer");

    logsimple("Cleanup rt");
    fsarr_free(&rt->body);
    if (rt->fl)
        fclose(rt->fl), rt->fl = 0;
    if (rt->runfl)
        fclose(rt->runfl), rt->runfl = 0;
    rt->flname = rt->runflname = 0; // must be static or full program live! No free here
    lexem_free(&rt->lex);
    logsimple("Clean rt done");
}

bool                            addline(Runtimedata *restrict rt, fs str){
    invraise(rt != 0, "Null pointer");

    // MOVE fs into fsarray
    fsarray     *b = &rt->body;
    if (rt->bodyptr >= fsarr_cnt(b) )
        if (fsarr_increase(b, rt->bodyptr + 100) < 0)
            return userraise(false, ERR_UNABLE_ALLOCATE, "Unable to add lines to body");
    fs_cpy(b->ar + rt->bodyptr++, str);
    return logsimpleret(true, "Added");
}

