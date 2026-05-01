


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
static const int        ARR_MAX = 12;
static const int        ARR_MAX = 1024 * 1024 * 64;    // 64mb

static char             g_arr[ARR_MAX][ARR_MAX];

