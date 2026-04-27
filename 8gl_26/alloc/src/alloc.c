#include <unistd.h>

#include "alloc.h"
#include "fs.h"
#include "log.h"
#include "checker.h"
#include "guard.h"

static const int        NALLOC = 1024;
static Header           base;
static Header          *freep = 0;
static unsigned         totalalloc = 0; 

static Header               *morecore(unsigned);
static void                  printnode(FILE *restrict out, const Header *restrict p, int *restrict cnt);
void                         afree(void *);

void                        *alloc(unsigned nbytes){
    logenter("%u", nbytes);
    Header      *p, *prevp;
    unsigned     nunits;

    nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;
    logauto(nunits);
    if ((prevp = freep) == 0){      // init
        base.ptr = freep = prevp = &base;
        base.size = 0;
    }
    for (p = prevp->ptr; ; prevp = p, p = p->ptr){
        if (p->size >= nunits) {
            if (p->size == nunits)
                prevp->ptr = p->ptr;
            else {
                p->size -= nunits;
                p += p->size;
                p->size = nunits;
            }
            freep = prevp;
            return (void *) (p + 1);
        }
        if (p == freep)
            if ((p = morecore(nunits) ) == 0)
                return logerr( (void *) 0, "Unable to obtain");
    }
}

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

unsigned                     atotal(void){
    return totalalloc;
}

int                          afprint_all(FILE *out){
    int          cnt = 0;
    Header      *p, *prevp;

    printf("Total %u/%lu\n", atotal(), atotal() * sizeof(Header));
    if (out){
        prevp = freep;  // &base ???
        bool first_run = true;
        for (p = prevp->ptr; (first_run || (!first_run && p != freep) ) && RGUARDK; prevp = p, p = p->ptr){
            first_run = false;
            printnode(out, p, &cnt);
        }
    }
    return cnt;
}

static void                  printnode(FILE *out, const Header *restrict p, int *restrict cnt){
    fprintf(out, "Node %4d: sz %u, off %lu\n", *cnt++, p->size, p->ptr - freep);
}

