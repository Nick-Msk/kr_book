#ifndef _FS_ARRAY_H
#define _FS_ARRAY_H

// ---------------------------------------------------------------------------------------
// --------------------------- Public Faststring Array API -------------------------------
// ---------------------------------------------------------------------------------------

// --------------- Includes -----------------------

#include <stdio.h>
#include <string.h>
//
#include "bool.h"
#include "log.h"
#include "common.h"
#include "error.h"

// ------------------- CONSTANTS AND GLOBALS ---------------------------

// --------------------------------- TYPES -----------------------------

// type-support functions

// ------------------------ CONSTRUCTOTS/DESTRUCTORS -------------------

// returns count of freed
int            fsarr_free(fsrr *arr);

// -------------------- ACCESS AND MODIFICATORS ------------------------

// ------------------------ PRINTERS/CHECKERS --------------------------

// ------------------------------ ETC. ---------------------------------

#endif /* !_FS_ARRAY_H */
