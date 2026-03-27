#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "log.h"
#include "common.h"
#include "fs.h"
#include "fs_iter.h"
#include "error.h"
#include "buffer.h"

typedef struct Keys {
    bool    version;
    bool    sens;
    // ...
} Keys;
#define                 Keysinit(...) (Keys){ .version = false, .sens = true, __VA_ARGS__}

typedef struct tnode {
    fs                 word;
    int                cnt;
    struct tnode      *left;
    struct tnode      *right;
} tnode;

