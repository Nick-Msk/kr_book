
#include "alloc2.h"
#include "fs.h"
#include "log.h"
#include "checker.h"
#include "guard.h"

typedef long Align;

typedef union Header {
    struct {
        union Header   *freeptr;
        unsigned        size;
    };
    Align               not_used;
} Header;

#define                 HeaderInit(...) (Header) {.freeptr = 0, .size = 0, ##__VA_ARGS__ }

typedef struct Control {
    Header             *freeptr;   // to g_arr + i etc
    unsigned            total;  // units!!!
    unsigned            free;   // units!!!
} Control;

// storage
static const int        ARR_MAX_CNT     = 12;
static const int        ARR_MAX         = 1024 * 1024;    // 1mb of Header
static const int        ARR_MAX_UNIT    = ARR_MAX / sizeof(Header);

// never user that array directly, only via control structure
static Header          g_arr[ARR_MAX_CNT][ARR_MAX]; // to avoid using mmap()
static int              g_ptr;  // pointer to current unallocated!

#define                 ControlInit(N) (Control) {.freeptr = (Header *)g_arr + ARR_MAX_UNIT * (N), .total = ARR_MAX_UNIT, .free = ARR_MAX_UNIT }

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

static void                 initlocation(Header *ptr, unsigned sz){
    *(Header *) ptr = HeaderInit(.freeptr = ptr + 1, .size = sz - 1);    // -1 for header
}

static inline bool          checklimitlocation(Header *ptr, int loc){
    return (char *) ptr < (char *) g_control[loc].freeptr + g_control[loc].total;
}

static void                 updatelocation(int loc, unsigned cntu, bool alloc){
    if (alloc)  // -=
        g_control[loc].free -= cntu;
    else
        g_control[loc].free += cntu;
    invraise(g_control[loc].free <= g_control[loc].total, "Out of range free %u: total %d", g_control[loc].free, g_control[loc].total);
}

static inline unsigned      calc_units(unsigned bytes){
    return (bytes + sizeof(Header) - 1) / sizeof(Header) + 1;
}

// loc < 12
static void                *findlocation(int loc, unsigned bytes){
    invraise(loc >= 0 && loc < ARR_MAX_CNT, "Out of loc %d", loc);

    logenter("loc %d: %u", loc, bytes);
    // converting into units
    unsigned nu = calc_units(bytes);
    logauto(nu);

    if (g_ptr == loc)       // 1-st alloc this, need to formar
        initlocation(g_control[loc].freeptr, ARR_MAX_UNIT), g_ptr++;

    if (g_control[loc].free >= nu){

        Header         *pos = g_control[loc].freeptr;
        Header         *prev = 0;
        for (; pos && checklimitlocation(pos, loc); prev = pos, pos = pos->freeptr){
            if (pos->size >= nu){
                if (pos->size == nu){       // exact!
                    if (prev)
                        prev->freeptr = pos->freeptr;
                    else        // 1-st iter, use g_control[loc].ptr instead
                        g_control[loc].freeptr = pos->freeptr;
                } else  {   // alloc area at the and of free space TODO: 

                
                }
                // setup Header *pos
                pos->size = nu;     // to check in afree()
                updatelocation(loc, nu, true);
                return logret( (void *) (pos + 1), "Allocated %u", nu);  // header remains
            }
        }

    }
    return logerr( (void *) 0, "Unable to alloc %u units", nu);
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

