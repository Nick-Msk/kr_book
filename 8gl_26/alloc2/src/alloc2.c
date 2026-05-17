
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
static const int        ARR_MAX_BYTES   = 1024 * 1024;    // 1mb of Header
static const int        ARR_MAX_UNIT    = ARR_MAX_BYTES / sizeof(Header);

// TODO: rework that to get_osmap(sz);
// never user that array directly, only via control structure
static Header           g_arr[ARR_MAX_CNT][ARR_MAX_UNIT]; // to avoid using mmap()
static int              g_ptr =         0;  // pointer to current unallocated!

#define                 CalcArrPtr(N) (Header *) g_arr + ARR_MAX_UNIT * (N)

#define                 ControlInit(N) (Control) {.freeptr = CalcArrPtr(N), .baseptr = CalcArrPtr(N), .total = ARR_MAX_UNIT, .free = ARR_MAX_UNIT }

static Control          g_control[ARR_MAX_CNT] =
{   ControlInit(0) };

// ------------------------------- Utilities -------------------------------------


#define                     foralllocs(iter) for (int iter = 0; i < ARR_MAX_CNT; i++)

static int                  fprintheader(FILE *restrict out, const Header *restrict ph);
static inline int           printheader(const Header *p);
static int                  fprintfreeheader(FILE *restrict out, const Header *restrict ph);
static int                  printfreeheader(const Header * ph);
static void                 init_control(int loc);

// --------------------------- Location API --------------------------------------

static Header              *getlastptr(int loc){
    invraise(loc >= 0 && loc < ARR_MAX_CNT, "Invalid location %d", loc);
    return g_control[loc].baseptr + g_control[loc].total;
}

// actually it's possible to binary_search here!
static int                 findlocation(const void *p){
    const Header *h = p;
    for (int i = 0; i < ARR_MAX_CNT; i++)
        if (h >= g_control[i].baseptr && h < getlastptr(i) )  // found!
            return logsimpleret(i, "Found loc %d", i);
    return logsimpleerr(-1, "Wrong pointer %p", p);
}

static inline Header       *locbaseptr(int loc){
    return g_control[loc].baseptr;
}

static inline Header       *getlocbaseptr(const void *p){
    int loc = findlocation(p);
    return loc >= 0 ? locbaseptr(loc) : 0;
}

static inline Header       *locfreeptr(int loc){
    return g_control[loc].freeptr;
}

static inline Header       *getlocfreeptr(const void *p){
    int loc = findlocation(p);
    return loc >= 0 ? locfreeptr(loc) : 0;
}

static inline unsigned     locfree(int loc){
    return g_control[loc].free;
}

static inline unsigned     getlocfree(const void *p){
    int loc = findlocation(p);
    return loc >= 0 ? locfree(loc) : -1;
}

static inline unsigned     loctotal(int loc){
    return g_control[loc].total;
}

static inline unsigned     getloctotal(const void *p){
    int loc = findlocation(p);
    return loc >= 0 ? loctotal(loc) : -1;
}

static void                 initlocation(Header *restrict ptr, unsigned sz, Header *restrict nextptr){
    invraise(ptr != 0 && sz > 1, "Invalid input %p, %u", ptr, sz);
    *(Header *) ptr = HeaderInit(.freeptr = nextptr, .size = sz);    // -1 for header upd: removed that rule
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
// some validity tests
static bool                acheckstructure_loc(int loc){
    invraise(loc >= 0 && loc < ARR_MAX_CNT, "Invalid location %d", loc);
    int             i = 0;
    const Header   *p, *prev = 0;
    for (p = locfreeptr(loc); p != 0x0; i++, prev = p, p = p->freeptr){
        // check p < p->next
        if (p->freeptr != 0 && p > p->freeptr)    // violation
            return logsimpleerr(false, "I. Reordered oointer violation loc %d on iter %d, %p > %p", loc, i, p, p->freeptr);
        if (prev && prev + prev->size == p) // that should NOT be!
            return logsimpleerr(false, "II. Neighbors free blocks violation loc %d on iter %d, %p ->freepre == %p", loc, i, prev, p); 
    }
    return logsimpleret(true, "Location %d ok", loc);
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
        init_control(loc);
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
        cnt += fprintf(out, "FREE BLK: %p  Pos %lu, size %u, ptr %p\t", ph, ph - getlocbaseptr(ph), ph->size, ph->freeptr);
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
        cnt += fprintf(out, "Pos %lu, size %u, ptr %p\t", h - getlocbaseptr(ph), h->size, h->freeptr);
    }
    return cnt;
}

static inline int           printheader(const Header *p){
    return fprintheader(stdout, p);
}

static void                 init_control(int loc){
    invraise(loc >= 0 && loc < ARR_MAX_CNT, "Out of loc %d", loc);
    g_control[loc] = ControlInit(loc);
    // (re)init memory
    initlocation(g_control[loc].baseptr, g_control[loc].total, 0x0);
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
    for (int i = 0; i < g_ptr; i++)
        init_control(i);
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

void                         afree(void *pv){
    invraise(pv != 0x0, "Null pointer p");

    int         loc;
    Header     *hp, *p = 0, *var = (Header *) pv - 1, *base = locbaseptr(loc = findlocation(var) );
    unsigned    nu =  var->size;
    logsimple("loc %d, nu %u", loc, nu);

    for (hp = base; hp && hp < var; p = hp, hp = hp->freeptr)
        ;
    logsimple("diff base %lu, diff prev %lu, diff hp %lu", var - base, var - p, p->freeptr - var);

    if (var + var->size == p->freeptr){    // up // p->freeptr > var
        logsimple("up, bp.sz %u + next p.sz %u", var->size, p->freeptr->size);
        var->size += p->freeptr->size;
        var->freeptr = p->freeptr->freeptr;
    } else
        var->freeptr = p->freeptr;

    logsimple("p.size %u, var - p %lu ", p->size, var - p);
    if (p + p->size == var){  // down, p < var
        logsimple("down, p.sz %u + bp.sz %u", p->size, var->size);
        p->size += var->size;
        p->freeptr = var->freeptr;
    } else {
        logsimple("before %p", p->freeptr);
        p->freeptr = var;
        logsimple("after remap to var %p", p->freeptr);
    }

    updatelocation(loc, nu, false); // + nu to control
}

// sub
unsigned                      agetallocatedsize(const void *p){
    const Header *h = p;
    return (--h)->size * sizeof(Header);
}
// sub
bool                          acheckstructure(void){
    bool res = true;
    for (int i = 0; i < g_ptr; i++){
        res &= acheckstructure_loc(i);
    }
    return logsimpleret(res, "Structure status %s", bool_str(res) );
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

        atechfprint(logfile);

        unsigned res_after = acalcfreespace(), totalalloc = agetallocatedsize(s1) / sizeof(Header) +
                                                            agetallocatedsize(s2) / sizeof(Header) +
                                                            agetallocatedsize(s3) / sizeof(Header) +
                                                            agetallocatedsize(s4) / sizeof(Header);
        logmsg("1 byte alloc: %lu", agetallocatedsize(s3) / sizeof(Header));
        logmsg("16 byte alloc: %lu", agetallocatedsize(s1) / sizeof(Header));
        test_validate(res_after == ARR_MAX_UNIT - totalalloc,
                "Free space %u Must be equal %u", res_after, ARR_MAX_UNIT - totalalloc);

        test_validatefree(acheckstructure(), areset(), "Validation vailed");
    }
    test_sub("subtest %d: reset", ++subnum);
    {
        areset();
        unsigned res = acalcfreespace();
        test_validate(res == ARR_MAX_UNIT, "Must be %u after areset but not %u", ARR_MAX_UNIT, res);
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
        /*
        unsigned res_before = acalcfreespace();
        char    *s1 = alloc(ARR_MAX_BYTES + 1); // more that ARR_MAX_UNIT!
        test_validate(s1 == 0x0, "Must not be allocated! (actual %p)", s1);
        unsigned res_after = acalcfreespace();
        test_validate(res_before * ARR_MAX_CNT == res_after, "%u * ARR_MAX_CNT must be equal %u (after failed alloc)",
                res_before * ARR_MAX_CNT, res_after);
        areset();  DISABLED */
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------- TEST 3 ---------------------------------

typedef struct t_alloc {
    char        *s;
    int          sz;
} t_alloc;

#define CHECK_ALLOCATION(STAGE, ...) { t_alloc *val =(t_alloc []) { __VA_ARGS__, { 0x0, -1} };\
                                t_alloc *iter = val;\
                                int      loc = findlocation(val->s);\
                                unsigned total_alloc = 0U;\
                                unsigned free_blks = acalcfreespace_loc(loc);\
                                unsigned total = loctotal(loc);\
                                while (iter->s){\
                                    unsigned curr_alloc = agetallocatedsize(iter->s);\
                                    total_alloc += curr_alloc;\
                                    test_validatefree(iter->sz + 2 * sizeof(Header) >= curr_alloc && curr_alloc >= iter->sz + sizeof(Header)\
                                            , areset()\
                                            , "Allocated value %u must not be more that %lu of requested value %u"\
                                            , curr_alloc, sizeof(Header), iter->sz);\
                                    iter++;\
                                }\
                                total_alloc /= sizeof(Header);\
                                test_validatefree(acheckstructure(), areset(), "Validation vailed after " STAGE);\
                                test_validatefree(free_blks == total - total_alloc,  areset(),\
                                    STAGE " free = %u units, but must be %u", free_blks, total - total_alloc);\
                                test_validatefree(free_blks == locfree(loc), areset(),\
                                    STAGE "Total size of free units %u must be equal to control free size %u", free_blks, locfree(loc) );\
}

#define TVAR(a, b) (t_alloc) {.s = a, .sz = b }

// only for partial case!
#define CHECK_ALLOC_RESET(){ areset();\
        unsigned res =  acalcfreespace();\
        test_validate(res == ARR_MAX_UNIT, "after areset free = %u units, but must be %u", res, ARR_MAX_UNIT); }\

static TestStatus
tf3(const char *name)
{
    logenter("%s", name);
    int         subnum = 0;

    test_sub("subtest %d: alloc + afree simple", ++subnum);
    {
        char *s = alloc(123);
        test_validatefree(acheckstructure(), areset(), "Validation vailed");
        afree(s);
        atechfprint(logfile);

        unsigned res =  acalcfreespace();
        test_validatefree(res == ARR_MAX_UNIT,  areset(), "free = %u units, but must be %u", res, ARR_MAX_UNIT);
        test_validatefree(acheckstructure(), areset(), "Validation vailed");

        areset();
    }
    test_sub("subtest %d: alloc + afree simple", ++subnum);
    {
        char *s1 = alloc(444);
        char *s2 = alloc(13);
        char *s3 = alloc(188);

        test_validate(s1 && s2 && s3, "Fail to allocate %p, %p, %p", s1, s2, s3);

        // try to free
        afree(s1);
        atechfprint(logfile);

        unsigned res =  acalcfreespace();
        unsigned total_alloc = (agetallocatedsize(s2) + agetallocatedsize(s3) ) / sizeof(Header);

        test_validatefree(res == getloctotal(s2) - total_alloc,  areset(),
                "after free s1 free = %u units, but must be %u", res, getloctotal(s2) - total_alloc);
        test_validatefree(res == getlocfree(s2), areset(),
                "Total size of free units %u must be equal to control free size %u", res, getlocfree(s2) );

        //
        afree(s2);
        atechfprint(logfile);

        test_validatefree(acheckstructure(), areset(), "Validation vailed after free s2");
        total_alloc = ( agetallocatedsize(s3) ) / sizeof(Header);

        res =  acalcfreespace();

        test_validatefree(res == getloctotal(s2) - total_alloc,  areset(),
                "after free s2&&s1 free = %u units, but must be %u", res, getloctotal(s2) - total_alloc);
        test_validatefree(res == getlocfree(s3), areset(),
                "Total size of free units %u must be equal to control free size %u", res, getlocfree(s3) );

        CHECK_ALLOC_RESET();
    }
    test_sub("subtest %d: alloc + afree simple II", ++subnum);
    {
        int         sz1 = 444, sz2 = 13, sz3 = 188, sz4 = 17, sz5 = 210;
        char       *s1 = alloc(sz1);
        char       *s2 = alloc(sz2);
        char       *s3 = alloc(sz3);
        char       *s4 = alloc(sz4);
        char       *s5 = alloc(sz5);

        test_validate(s1 && s2 && s3 && s4 && s5, "Fail to allocate %p, %p, %p, %p, %p", 
                        s1, s2, s3, s4, s5);

        // try to free
        afree(s2);
        atechfprint(logfile);
        CHECK_ALLOCATION("After free s2", TVAR(s1, sz1), TVAR(s3, sz3), TVAR(s4, sz4), TVAR(s5, sz5) );

        afree(s4);
        atechfprint(logfile);
        CHECK_ALLOCATION("After free s4", TVAR(s1, sz1), TVAR(s3, sz3), TVAR(s5, sz5) );

        afree(s1);
        atechfprint(logfile);
        CHECK_ALLOCATION("After free s1", TVAR(s3, sz3), TVAR(s5, sz5) );

        afree(s5);
        atechfprint(logfile);
        CHECK_ALLOCATION("After free s5", TVAR(s3, sz3) );

        afree(s3);
        atechfprint(logfile);

        CHECK_ALLOC_RESET();    // exec areset() here
    }
    return logret(TEST_PASSED, "done"); // TEST_FAILED, TEST_PASSED, TEST_MANUAL
}

// ------------------------------------------------------------------------------------------------------------------------------
int
main( /* int argc, const char *argv[] */)
{
    logsimpleinit("Start");

    testenginestd(
        testnew(.f2 = tf1,  .num =  1, .name = "Simple alloc and validate test"             , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf2,  .num =  2, .name = "Allocate too much test"                     , .desc=""                , .mandatory=true)
      , testnew(.f2 = tf3,  .num =  3, .name = "Simple "                                    , .desc=""                , .mandatory=true)
    );
    return logret(0, "end...");  // as replace of logclose()
}

#endif /* ALLOC2TESTING */

