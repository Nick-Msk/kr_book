
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
static const int        ARR_MAX         = 1024; // for TEST!//1024 * 1024;    // 1mb of Header
static const int        ARR_MAX_UNIT    = ARR_MAX / sizeof(Header);

// TODO: rework that to get_osmap(sz);
// never user that array directly, only via control structure
static Header           g_arr[ARR_MAX_CNT][ARR_MAX]; // to avoid using mmap()
static int              g_ptr =         0;  // pointer to current unallocated!

#define                 CalcArrPtr(N) (Header *) g_arr + ARR_MAX_UNIT * (N)

#define                 ControlInit(N) (Control) {.freeptr = CalcArrPtr(N), .baseptr = CalcArrPtr(N), .total = ARR_MAX_UNIT, .free = ARR_MAX_UNIT }

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

    if (g_ptr == loc){       // 1-st alloc this, need to formar
        g_control[g_ptr] = ControlInit(g_ptr);
        initlocation(g_control[loc].baseptr, ARR_MAX_UNIT, 0x0);
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
void                        *alloc(unsigned bytes){
    void    *ret = 0;
    for (int i = 0; i < ARR_MAX_CNT; i++){
        ret = findmemory(i, bytes);
        if (ret)    // found
            return logsimpleret(ret, "Allocated %u", bytes);
    }
    return logsimpleerr(ret, "Out of mem %u", bytes);
}

void                         areset(void){
    for (int i = 0; i < ARR_MAX_CNT; i++){
        g_control[i].freeptr = g_control[i].baseptr;
        g_control[i].free = g_control[i].total;
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
    );
    return logret(0, "end...");  // as replace of logclose()
}

#endif /* ALLOC2TESTING */

