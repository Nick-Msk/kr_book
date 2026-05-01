#include <unistd.h>

#include "alloc.h"
#include "fs.h"
#include "log.h"
#include "checker.h"
#include "guard.h"

static const int             NALLOC = 1024;
static const unsigned        MAX_ALLOX = 65536 * 1024;
static Header                base;
static Header               *freep = 0;
static unsigned              totalalloc = 0;

static Header               *morecore(unsigned);
static void                  printnode(FILE *restrict out, const Header *restrict p, int *restrict cnt);

static Header               *morecore(unsigned nu){
    void        *cp;
    Header      *up;

    if (nu < NALLOC)
        logauto(nu = NALLOC);
    cp = sbrk(nu * sizeof(Header) );
    if (cp == (void *) - 1)
        return sysraise( (Header *) 0, "Sbrk failed");
    logauto(totalalloc += nu);
    up = (Header *) cp;
    up->size = nu;
    afree(up + 1);
    return logsimpleret(freep, "added %u, %p", nu, freep);
}

static void                  printnode(FILE *out, const Header *restrict p, int *restrict cnt){
    fprintf(out, "Node %4d-%p %s: sz %u, next free %p\n", (*cnt)++, p, p == &base ? ("base"): "", p->size, p->ptr);
}

// ------------------------------------ API ------------------------------------------------
void                        *alloct(unsigned bytes, unsigned size){
    Header *p = alloc(bytes * size);
    if (!p)
        return sysraise( (void *) 0, "Unable to custom alloc");
    memset(p, 0, bytes * size);
    return p;
}
// constructor
void                        *alloc(unsigned nbytes){
    logenter("%u", nbytes);

    if (nbytes > MAX_ALLOX)
        return logerr( (void *) 0, "Too big value %u (max %u)", nbytes, MAX_ALLOX);

    Header      *p, *prevp;
    unsigned     nunits;

    nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
    logauto(nunits);
    if ((prevp = freep) == 0){      // init
        base.ptr = freep = prevp = &base;
        logauto(base.size = 0);
    }
    for (p = prevp->ptr; ; prevp = p, p = p->ptr){
        if (p->size >= nunits) {
            if (p->size == nunits)
                prevp->ptr = p->ptr;
            else {
                logsimple("found p %p, p->size %d", p, p->size);
                p->size -= nunits;
                p += p->size;
                p->size = nunits;
            }
            logsimple("p offset = %lu\n", p - prevp);
            freep = prevp;
            return logret((void *) (p + 1), "Allocated");
        }
        if (p == freep)
            if ((p = morecore(nunits) ) == 0)
                return sysraise( (void *) 0, "Unable to obtain new mem");
    }
}
// destructor
void                         afree(void *ap){
    Header      *bp, *p;

    bp = (Header *) ap - 1;
    for (p = freep; !(bp > p && bp < p->ptr); p = p->ptr)
        if (p >= p->ptr && (bp > p || bp < p->ptr) )
            break;

    if (bp + bp->size == p->ptr){    // up
        logsimple("up, bp.sz %u + p.sz %u", bp->size, p->ptr->size);
        bp->size += p->ptr->size;
        bp->ptr = p->ptr->ptr;
    } else
        bp->ptr = p->ptr;
    if (p + p->size == bp){  // down
        logsimple("down, p.sz %u + bp.sz %u", p->ptr->size, bp->size);
        p->size += bp->size;
        p->ptr = bp->ptr;
    } else
        p->ptr = bp;
    logsimple("Freed %p -> %p", freep, p);
    freep = p;
}
// anyarray additionally adding
bool                         abfree(void *ptr, unsigned n){
    invraise(ptr != 0, "Null pointer");
    logenter("%p - %u", ptr, n);
    if (n < 2 * sizeof(Header) || n > INT_MAX)    // negatove
        return logerr(false, "To big or too small piece %u", n);
    Header      *bp = (Header *) ptr, *p;

    n -= (n % sizeof(Header) );     // normallization
    bp->size = n / sizeof(Header) - 1;

   // OMG, just exec afree(bp + 1) from here ))))))

    logauto(bp->size);
    for (p = freep; !(bp > p && bp < p->ptr); p = p->ptr)
        if (p >= p->ptr && (bp > p || bp < p->ptr) )
            break;
    logmsg("p %p p->ptr %p", p, p->ptr);
    bp->ptr = p->ptr;
    p->ptr = bp;
    return logret(true, "Added %u units", bp->size);
}

unsigned                     atotal(void){
    return totalalloc;
}
// simple printer
int                          afprint_all(FILE *out){
    int          cnt = 0;
    Header      *p;

    if (out){
        fprintf(out, "Total %u/%lu\n", atotal(), atotal() * sizeof(Header));
        bool first_run = true;
        for (p = freep; (first_run || (!first_run && p != freep) ) && RGUARDM; p = p->ptr){
            first_run = false;
            printnode(out, p, &cnt);
        }
        if (cnt > 0)
            fprintf(out, "Total free nodes %d\n", cnt);
    }
    return cnt;
}

