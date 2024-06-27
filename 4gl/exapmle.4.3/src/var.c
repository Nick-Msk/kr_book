#include "var.h"
#include <ctype.h>
#include <stdio.h>

static const int    VARSZ = 'z' - 'a' + 1 + 1;

enum { VAR_UNSET = 0, VAR_SET = 1, VAR_ARR = 2} TypeVar;

struct tvar {
    union {
        double      value;
        struct {
            int      size;
            double  *arr;
        };
    };
    TypeVar     flags;   // arr pointed to memory ONLY  if VAR_ARR
};

// INTERNAL API
static tvar        *var_get(char c);
static int          var_getind(char c);

static struct tvar         vals[VARSZ];

static bool         var_checkname(const char *name, bool print){
    char c = *name;
    if (name[1] != '\0' || !(isalpha(c) || c == '?' )){
        if (print)
            fprintf(stderr, "Incorrect variable %s(%c)\n", name, c);
        return false;
    }
    return true;
}

static bool         var_checkarray(const char *name, bool print){
    if (!var_checkname(name, print))
        return false;
    if (var_get(name[0]->flags != VAR_ARR)){
        if (print)
            fprintf(stderr, "%s isn't array\n", name);
    }
    return true;
}

// (MUST exists, no checking here!)
static int          var_getind(char c){
    c = tolower(c);
    if (c == '?')
        return VARSZ - 1;
    else
        return c - 'a'; // from 0 till 25
}

// get pointer to tvar by name (MUST exists, no checking here!)
static tvar        *var_get(char c){
    return vals + var_getind(c);
}

// TODO: rework to use  var_set("b") = 1.123;
bool                var_set(const char *name, double val){
    if (!var_checkname(name, true))
        return false;
    struct tvar t = {val, 1};
    vals[var_getind(name[0])] = t;
    return true;
}

bool                var_isset(const char *name){
    if (!var_checkname(name, true))
        return false;
    return vals[var_getind(name[0])].flags == 1;
}

bool                var_unset(const char *name){
    if (!var_checkname(name, true))
        return false;
    // free if this is array
    if (vals[var_getind(name)] == VAR_ARR)  // TODO: reword via get_var()
        free(vals[var_getind(name)].arr;
    struct tvar t = {.value = 0.0, flags = VAR_UNSET};
    vals[var_getind(name[0])] = t;
    return true;
}

double              var_get(const char *name){ // 0.0 if not exists 
    if (!var_checkname(name, true))
        return 0.0;
    else {
        if (vals[i].flags == VAR_SET)
            return vals[var_getind(name[0])].value;
        else {
            fprintf(stderr, "%s isn't variable\n", name);   // rework to use standard array checking
            return 0.0;
        }
    }
}

void                var_clean(void){
    for (int i = 0; i < VARSZ; i++){
        if (vals[i].flags == VAR_ARR)       // TODO: use var_isarray() here
            free(vals[i].arr;
        vals[i].value = 0.0;
        vals[i].flags = VAR_UNSET;
    }
}

int                 var_getarrsize(const char *name){
    if (!var_checkarray(name, true))
        return -1;
    else
        return vals[i].size;
}

double             *var_getarr(const char *name){
    if (!var_checkarray(name, true))
        return 0;
    else
        return vals[i].arr;
}

bool                var_setarray(const char *name, int size){
    if (!var_checkname(name, true))
        return false;
    struct tvar t;
    t.flags = VAR_ARR;
    if ((t.arr = malloc(size * sizeof(double))) == 0){
        fprintf(stderr, "Unable to allocate %d items for %s\n", size, name);
        return false;
    }
    t.size = size;
    vals[
}

// ------------------------------- Testing ----------------------------------
#ifdef VARTESTING

#include "test.h"

// --------------------------------- TEST 1 ---------------------------------

// Bare  test set/check/unset
static TestStatus           tf1(void)
{
    logenter("%s: Bare test set/check/unset", __func__);

    double  v = var_get("a");
    if (v != 0.0)
        return logret(TEST_FAILED, "v = %f but must be = 0.0", v);

    var_set("b", 5.5);
    if ((v = var_get("b")) != 5.5)
        return logret(TEST_FAILED, "v = %f but must be = 5.5", v);
    if (!var_unset("b"))
        return logret(TEST_FAILED, "'b' was set, but var_unset returned false");

    return logret(TEST_PASSED, "done");
}

// ---------------------------------------------------------------------------
int                         main(int argc, char *argv[])
{
    LOG(const char *logfilename = "log/var.log");   // TODO: rework that! It should be simple

    if (argc > 1)
        logfilename = argv[1];

    loginit(logfilename, false,  0, "Starting");    // TODO: should be loginitsimple("Starting")

        testenginestd(
            testnew(.f2 = tf1, .num = 1, .name = "Base test", .desc = "Bare test set/check/unset"				, .mandatory=true)
          /*, testnew(.f2 = tf2, .num = 2, .name = "Several error raise test"	, .desc = "Several test (via macro, with junt retcode = 0).", .mandatory=true)
          , testnew(.f2 = tf3, .num = 3, .name = "Interrupt raising test"	, .desc = "Exception test."									, .mandatory=true)
		  , testnew(.f2 = tf4, .num = 4, .name = "System error test" 		, .desc = "System error raising test (w/o exception)."		, .mandatory=true)
          , testnew(.f2 = tf5, .num = 5, .name = "Return with action test"  , .desc = "Just raise with ACTION."							, .mandatory=true)
		  , testnew(.f2 = tf6, .num = 6, .name = "Default handler test"  	, .desc = "Trying default handler."                         , .mandatory=true)
		  , testnewa(.f2 = tf7, .num = 7, .name = "Try + catch local  test"	, .desc = "Full local cycle with catching and longjmp."     , .mandatory=true)*/
        );

    logclose("end...");
    return 0;
}


#endif /* VARTESTING */

