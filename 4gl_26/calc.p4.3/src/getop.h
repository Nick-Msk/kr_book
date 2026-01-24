#ifndef _GETOP_H
#define _GETOP_H

// ---------------------------------------------------------------------------------
// --------------------------- Public GETOP API ----------------------------------
// ---------------------------------------------------------------------------------

// ----------- CONSTANTS AND GLOBALS ---------------

static const        char LEXIC_NUMBER = '0';  // tp be removed

// ------------------- TYPES -----------------------

// ------------- CONSTRUCTOTS/DESTRUCTORS ----------

// TODO:

// -------------- ACCESS AND MODIFICATION ----------

extern int              lexic_getop(char *s, int sz);
extern void             lexic_clear(void);

// ----------------- PRINTERS ----------------------

extern int              lexic_fprint(FILE *f);

static inline int       lexic_print(void){
    return lexic_fprint(stdout);
}

// ------------------ ETC. -------------------------

#endif /* !_GETOP_H */
