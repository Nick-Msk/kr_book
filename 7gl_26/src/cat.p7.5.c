#include <stdio.h>
#include <string.h>

#include "log.h"
#include "common.h"
#include "error.h"

const char *usage_str = "Usage: %s <val1:TYPE1> <val2:TYPE2>\n";

int                     main(int argc, const char *argv[]){
    logsimpleinit("Start");

    

    return logret(0, "end...");  // as replace of logclose()
}

