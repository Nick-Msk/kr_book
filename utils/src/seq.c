/********************************************************************
                    SEQUENCE MODULE IMPLEMENTATION
********************************************************************/

#include "seq.h"

enum { SEQ_MAXCOUNT = 128, G_TECHPTINTPERLINE = 10 };        // for now

// == 0 means dropped or not init
static seqv_t        sequences[SEQ_MAXCOUNT] = {0};
static seqnum_t      sequencesptr = 0;

// ------------------------------------ Utilities ------------------------------------------


int                 findseq(void) {
    for (seqnum_t i = 0; i < sequencesptr; i++)
        if (sequences[i])   // found dropped
            return logsimpleret(i, "Found %d", i);
    return logsimpleret(-1, "Unable to find empty seq");
}

// ------------------------------------- API -----------------------------------------------

seqnum_t            initseq(void) {
    seqnum_t     num;
    if (sequencesptr >= SEQ_MAXCOUNT) {
        if ( (num = findseq() ) < 0)
            userraiseint(ERR_UNABLE_ALLOCATE_SEQ, "Unable to allocate sequence");   // probably need to be configurable in context
    } else
        num = sequencesptr++;
    sequences[num] = 1L;    // init
    return num;
}

void                dropseq(seqnum_t s) {
    if (! (s >= 0 && s < SEQ_MAXCOUNT) )
        userraiseint(ERR_OUT_OF_RANGE, "%d must be positive and < %d", s, SEQ_MAXCOUNT);
    sequences[s] = 0L;    // un init
}

seqv_t              currval(seqnum_t s) {
    if (! (s >= 0 && s < SEQ_MAXCOUNT) )
        userraiseint(ERR_OUT_OF_RANGE, "%d must be positive and < %d", s, SEQ_MAXCOUNT);
    return sequences[s];
}

seqv_t           nextval(seqnum_t s) {
    if (! (s >= 0 && s < SEQ_MAXCOUNT) )
        userraiseint(ERR_OUT_OF_RANGE, "%d must be positive and < %d", s, SEQ_MAXCOUNT);
    return ++sequences[s];
}

// ------------------------ Printers ---------------------------------------
int                 seq_techfprint(FILE *out) {
    int     cnt = 0;
    if (out) {
        int total = 0;
        fprintf(out, "SEQ_T: %d/%d [\n", sequencesptr, SEQ_MAXCOUNT);
        for (int i = 0; i < sequencesptr; i++) {
            if (sequences[i])
                fprintf(stdout, "%3d:%6lld\t", i, sequences[i]), total++;
        }
        fprintf(out, "], total allocated %d\n", total);
    }
    return cnt;
}

// ---------------------------------------- Testing ------------------------------------------
#ifdef SEQTESTING

#include "test.h"

// ------------------------- TEST value64_tarray ---------------------------------
static TestStatus
tf(const char *name)
{
    logenter("%s", name);
    int subnum = 0;

    return logret(TEST_PASSED, "done");
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main(int argc, const char *argv[])
{
    logsimpleinit("Start");
    bool    runall = argc == 1;

    while (runall || *++argv){
        int     num = INT_MAX;    // INT_MAX for all test
        if (!runall){
            num = atoi(*argv);
            if (num < 0){
                fprintf(stderr,"Invalid test num %d\n", num);
                continue;
            }
        }
        testenginestd_run(num,
            testnew(.f2 =  tf,                                 .num = 1, .name = "tf_value64_tarray"
                , .desc="", .mandatory=true)
        );
        if (runall)
            break;
    }

    return logret(0, "end...");  // as replace of logclose()
}

#endif /* SEQTESTING */


