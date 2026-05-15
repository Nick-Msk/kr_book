
#include "alloc2.h"
#include "fs.h"
#include "log.h"
#include "checker.h"
#include "guard.h"

typedef long Align;

#ifdef ALLOC_USE_NAME
static const int                ALLOC_MAX_NAME = 16;
#endif /* ALLOC_USE_NAME */

typedef union Header {
    struct {
        union Header   *freeptr;
        unsigned        size;
#ifdef ALLOC_USE_NAME
        char            name[ALLOC_MAX_NAME]; // for future use!
#endif /* ALLOC_USE_NAME */
    };
    Align               not_used;
} Header;

#define                 HeaderInit(...) (Header) {.freeptr = 0, .size = 0, ##__VA_ARGS__ }

typedef struct Control {
    Header             *freeptr;   // to g_arr + i etc
    Header             *baseptr;   // const!
    unsigned            total;  // const units!!!
    unsigned            free;   // units!!!
} Control;

// storage
static const int        ARR_MAX_CNT     = 12;
static const int        ARR_MAX_BYTES   = 1024; // for TEST!//1024 * 1024;    // 1mb of Header
static const int        ARR_MAX_UNIT    = ARR_MAX_BYTES / sizeof(Header);

// TODO: rework that to get_osmap(sz);
// never user that array directly, only via control structure
static Header           g_arr[ARR_MAX_CNT][ARR_MAX_UNIT]; // to avoid using mmap()
static int              g_ptr =         0;  // pointer to current unallocated!

#define                 CalcArrPtr(N) (Header *) g_arr + ARR_MAX_UNIT * (N)

#define                 ControlInit(N) (Control) {.freeptr = CalcArrPtr(N), .baseptr = CalcArrPtr(N), .total = ARR_MAX_UNIT, .free = ARR_MAX_UNIT - 1 }

static Control          g_control[ARR_MAX_CNT] =
{   ControlInit(0)
    /*ControlInit(1),
    ControlInit(2),
    ControlInit(3),
    ControlInit(4),
    ControlInit(5),
    ControlInit(6),
    ControlInit(7),
    ControlInit(8),
    ControlInit(9),
    ControlInit(10),
    ControlInit(11)*/
};

// ------------------------------- Utilities -------------------------------------

// --------------------------- Location API --------------------------------------

#define                     foralllocs(iter) for (int iter = 0; i < ARR_MAX_CNT; i++)

static int                  fprintheader(FILE *restrict out, const Header *restrict ph);
static inline int           printheader(const Header *p);
static int                  fprintfreeheader(FILE *restrict out, const Header *restrict ph);
static int                  printfreeheader(const Header * ph);

static Header              *getlastptr(int loc){
    invraise(loc >= 0 && loc < ARR_MAX_CNT, "Invalid location %d", loc);
    return g_control[loc].baseptr + g_control[loc].total;
}

// actually it's possible to binary_search here!
static int                 getlocation(const void *p){
    const Header *h = p;
    for (int i = 0; i < ARR_MAX_CNT; i++)
        if (h >= g_control[i].baseptr && h < getlastptr(i) )  // found!
            return logsimpleret(i, "Found loc %d", i);
    return logsimpleerr(-1, "Wrong pointer %p", p);
}

static inline Header       *getlocationbaseptr(const void *p){
    int loc = getlocation(p);
    return loc >= 0 ? g_control[loc].baseptr : 0;
}

static void                 initlocation(Header *restrict ptr, unsigned sz, Header *restrict nextptr){
    invraise(ptr != 0 && sz > 1, "Invalid input %p, %u", ptr, sz);
    *(Header *) ptr = HeaderInit(.freeptr = nextptr, .size = sz - 1);    // -1 for header
}

static inline bool          checklimitlocation(Header *ptr, int loc){
    return (char *) ptr < (char *) g_control[loc].freeptr + g_control[loc].total;
}

static void                 updatelocation(int loc, unsigned cntu, bool alloc){
    invraise(loc >= 0 && loc < ARR_MAX_CNT, "Invalid location %d", loc);
    if (alloc)  // -=
        g_control[loc].free -= cntu;
    else
        g_control[loc].free += cntu;
    invraise(g_control[loc].free <= g_control[loc].total, "Out of range free %u: total %d", g_control[loc].free, g_control[loc].total);
}

static int                  fprintheaderlist(FILE *restrict out, const Header *hlist){
    int     cnt = 0;
    if (out){
        while (hlist){
            cnt += fprintfreeheader(out, hlist);
            hlist = hlist->freeptr;
        }
    }
    return cnt;
}

static int                  fprintlocation(FILE *out, int loc){
    invraise(loc >= 0 && loc < ARR_MAX_CNT, "Invalid location %d", loc);
    int         cnt = 0;
    if (out){
        const Control *c = g_control + loc;
        cnt += fprintf(out, "LOCATION %d: base %p, free %p, delta %lu, total %u, free %u\n",
                loc, c->baseptr, c->freeptr, c->freeptr - c->baseptr, c->total, c->free);
        cnt += fprintheaderlist(out, c->freeptr);
        cnt += fprintf(out, "\n");
    }
    return cnt;
}

static unsigned             acalcfreespace_loc(int loc){
    unsigned    res = 0;
    for (const Header *ph = g_control[loc].freeptr; ph != 0; ph=ph->freeptr)
        res += ph->size;
    invraise(res == g_control[loc].free, "sum %u for %d must be equal total free %u", res, loc, g_control[loc].free);
    return logsimpleret(res, "%u for location %d", res, loc);
}

// --------------------------- Common Utilities ----------------------------------------------
static inline unsigned      calc_units(unsigned bytes){
    return (bytes + sizeof(Header) - 1) / sizeof(Header) + 1;
}

// loc < 12
static void                *findmemory(int loc, unsigned bytes){
    invraise(loc >= 0 && loc < ARR_MAX_CNT, "Out of loc %d", loc);

    logenter("loc %d: %u", loc, bytes);
    // converting into units
    unsigned nu = calc_units(bytes);
    logauto(nu);

    if (g_ptr == loc){       // 1-st alloc this, need to format
        // TODO: refactor that to init_control(loc)
        g_control[g_ptr] = ControlInit(g_ptr);
        initlocation(g_control[loc].baseptr, g_control[loc].total, 0x0);
        g_ptr++;
    }

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
                    pos->size -= nu;
                    pos += pos->size;
                    pos->size = nu; // new piece
                }
                pos->freeptr = 0;   // just for clean
                // setup Header *pos
                updatelocation(loc, nu, true);
                return logret( (void *) (pos + 1), "Allocated %u", nu);  // header remains
            }
        }

    }
    return logerr( (void *) 0, "Unable to alloc %u units", nu);
}

static int                  fprintfreeheader(FILE *restrict out, const Header *restrict ph){
    int     cnt = 0;
    if (out)
        cnt += fprintf(out, "FREE BLK: Pos %lu, size %u, ptr %p\t", ph - getlocationbaseptr(ph), ph->size, ph->freeptr);
    return cnt;
}

static int                  printfreeheader(const Header * ph){
    return fprintfreeheader(stdout, ph);
}

static int                  fprintheader(FILE *restrict out, const Header *restrict ph){
    int     cnt = 0;
    if (out){
        const Header    *h = (Header *) ph - 1;
#ifdef ALLOC_USE_NAME
        cnt += fprintf(out, "%s: ", h->name);      // not used for now
#endif
        cnt += fprintf(out, "Pos %lu, size %u, ptr %p\t", h - getlocationbaseptr(ph), h->size, h->freeptr);
    }
    return cnt;
}

static inline int           printheader(const Header *p){
    return fprintheader(stdout, p);
}

// ------------------------------------- API -------------------------------------

void                        *alloct(unsigned cnts, unsigned size){
    return alloc(cnts * size);
}

void                        *alloc(unsigned bytes){
    invraise(bytes <= ARR_MAX_BYTES * 4, "To heavy %u", bytes);
    void    *ret = 0;
    for (int i = 0; i < ARR_MAX_CNT; i++){
        ret = findmemory(i, bytes);
        if (ret)    // found
            return logsimpleret(ret, "Allocated %u", bytes);
    }
    return logsimpleerr(ret, "Out of mem %u", bytes);
}

void                         areset(void){
    for (int i = 0; i < g_ptr; i++){
        // TODO: init_control(i)
        g_control[i].freeptr = g_control[i].baseptr;
        initlocation(g_control[i].baseptr, g_control[i].total, 0x0);
        logauto(g_control[i].free = g_control[i].total - 1);
    }
    logsimple("Reset");
}

int                          atechfprint(FILE *restrict out){
    int     cnt = 0;
    if (out){
        fprintf(out, "G_PTR = %d\n", g_ptr);
        for (int i = 0; i < g_ptr; i++){
            cnt += fprintlocation(out, i);
        }
    }
    return cnt;
}

unsigned                     acalcfreespace(void){
    // got
    unsigned res = 0;
    for (int i = 0; i < g_ptr; i++){
        res += acalcfreespace_loc(i);
    }
    return logsimpleret(res, "Caclucated %u", res); 
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
        char    *s1 = alloc(25);
        printf("sizeof(Header) %lu, Header * %lu, unsigned %lu, long %lu\n", sizeof(Header), sizeof(Header *), sizeof(unsigned), sizeof(long) );
        printheader((const Header *)s1); // print structure Header by s - 1

        char    *s2 = alloc(18);
        printheader((const Header *)s2);

        char    *s3 = alloc(1);
        printheader((const Header *)s3);

        char    *s4 = alloc(111);
        printheader((const Header *)s4);
        fprintf(stdout, "\n");

        atechfprint(stdout);

        unsigned res_after = acalcfreespace();
        test_validate(res_after <= ARR_MAX_UNIT - 2 * 4 /* 4 units allocated */,  
                "Free space %u Must be less or equal %u", res_after, ARR_MAX_UNIT - 2 * 4);
    }
    test_sub("subtest %d: reset", ++subnum);
    {
        areset();
        unsigned res = acalcfreespace();
        test_validate(res == ARR_MAX_UNIT - 1, "Must be %u after areset but not %u", ARR_MAX_UNIT - 1, res);
    }
    return logret(TEST_MANUAL, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 2 ---------------------------------

static TestStatus
tf2(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: alloc many many", ++subnum);
    {
        unsigned res_before = acalcfreespace();
        char    *s1 = alloc(ARR_MAX_BYTES + 1); // more that ARR_MAX_UNIT!
        test_validate(s1 == 0x0, "Must not be allocated! (actual %p)", s1);
        unsigned res_after = acalcfreespace();
        test_validate(res_before * ARR_MAX_CNT == res_after, "%u * ARR_MAX_CNT must be equal %u (after failed alloc)", 
                res_before * ARR_MAX_CNT, res_after);
    }
    return logret(TEST_MANUAL, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main( /* int argc, const char *argv[] */)
{
    logsimpleinit("Start");

    testenginestd(
        testnew(.f2 = tf1,  .num =  1, .name = "Simple init and validate test"              , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf2,  .num =  2, .name = "Allocate too much test"                     , .desc=""                , .mandatory=true)
    );
    return logret(0, "end...");  // as replace of logclose()
}

#endif /* ALLOC2TESTING */

