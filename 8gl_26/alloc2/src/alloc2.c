


#include "alloc2.h"
#include "fs.h"
#include "log.h"
#include "checker.h"
#include "guard.h"



typedef union Header {
    struct {
        union Header   *ptr;
        unsigned        size;
    };
    Align       not_used;
} Header;


// storage
static const int        ARR_MAX_CNT = 12;
static const int        ARR_MAX     = 1024 * 1024 * 64;    // 64mb

static char             g_arr[ARR_MAX_CNT][ARR_MAX]; // to avoid using mmap()
static int             *g_ptr;  // pointer to current unallocate

static void             init_data(int sz){
    
}



