
#include "alloc2.h"
#include "fs.h"
#include "log.h"
#include "checker.h"
#include "guard.h"

typedef long Align;

typedef union Header {
    struct {
        union Header   *ptr;
        unsigned        size;
    };
    Align               not_used;
} Header;

typedef struct Control {
    void               *ptr;   // to g_arr + i etc
    size_t              total;
    size_t              free;
} Control;

// storage
static const int        ARR_MAX_CNT = 12;
static const int        ARR_MAX     = 1024 * 1024 * 64;    // 64mb

// never user that array directly, only via control structure
static char             g_arr[ARR_MAX_CNT][ARR_MAX]; // to avoid using mmap()
//static int             *g_ptr;  // pointer to current unallocated!

#define                 ControlInit(N) (Control) {.ptr = g_arr + ARR_MAX * (N), .total = ARR_MAX / sizeof(Header), .free = ARR_MAX / sizeof(Header) }

static Control          g_control[ARR_MAX_CNT] =
{   ControlInit(0),
    ControlInit(1),
    ControlInit(2),
    ControlInit(3),
    ControlInit(4),
    ControlInit(5),
    ControlInit(6),
    ControlInit(7),
    ControlInit(8),
    ControlInit(9),
    ControlInit(10),
    ControlInit(11)
};

// ------------------------------- Utilities -------------------------------------

// loc < 12
static void                *findlocation(int loc, unsigned bytes){
    invraise(loc >= 0 && loc < ARR_MAX_CNT, "Out of loc %d", loc);

    // converting into units
    unsigned nu = (bytes + sizeof(Header) - 1) / sizeof(Header) + 1;
    void        *ret = 0;

    if (g_control[loc].free >= nu){

        char        *pos = g_control[loc].ptr;
        logauto(nu);
        ret = pos;  // init

        // if found update fee
        if (ret)
            g_control[loc].free -= nu;
    }
    return logsimpleret(ret, "%u units allocated", nu);
}

// ------------------------------------- API -------------------------------------
void                        *alloc(unsigned bytes){
    void    *ret = 0;
    for (int i = 0; i < ARR_MAX_CNT; i++){
        ret = findlocation(i, bytes);
        if (ret)    // found
            return logsimpleret(ret, "Allocated %u", bytes);
    }
    return logsimpleerr(ret, "Out of mem %u", bytes);
}

// -------------------------------Testing --------------------------
#ifdef ALLOC2TESTING

#include "test.h"

//types for testing

// ------------------------- TEST 1 ---------------------------------

static TestStatus
tf1(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: alloc simple", ++subnum);
    {
        // TODO: 
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main( /* int argc, const char *argv[] */)
{
    logsimpleinit("Start");

    testenginestd(
        testnew(.f2 = tf1,  .num =  1, .name = "Simple init and validate test"              , .desc=""                , .mandatory=true)
    );
    return logret(0, "end...");  // as replace of logclose()
}

#endif /* ALLOC2TESTING */

