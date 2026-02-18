#ifndef _FS_ITER_H
#define _FS_ITER_H

// ---------------------------------------------------------------------------------
// --------------------------- Public Faststring API -------------------------------
// ---------------------------------------------------------------------------------

// --------------- Includes -----------------------

#include <stdio.h>
#include <string.h>
//
#include "bool.h"
#include "log.h"
#include "common.h"
#include "error.h"
#include "fs.h"

// ----------- CONSTANTS AND GLOBALS ---------------

// ------------------- TYPES -----------------------

// faststring iterator
typedef struct fs {
    int         len, sz; // sz >= len + 1 because of last '\0'
    FS_FLAGS    flags;
#if defined(FS_ALLOCATOR)
    int         pos;    //
#endif
    char       *v;      // with '\0'
} fs;

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

#define FSEMPTY (fs){.sz = 0, .len = 0, .flags = FS_FLAG_STATIC, .v = ""};

#define FSINITSTATIC(...)  (fs){.sz = 1, .len = 0, .flags = FS_FLAG_STATIC, .v = "", ##__VA_ARGS__}

#define FS(...)   (fs){.sz = 0, .len = 0, .flags = FS_FLAG_ALLOC, .v = 0, ##__VA_ARGS__}

#define fsfree(s) fs_free(&(s))

// -------------------- ACCESS AND MODIFICATORS ------------------------

// ------------------------ PRINTERS/CHECKERS --------------------------

// --------------------------- ETC. -------------------------

#endif /* !_FS_ITER_H */

