#ifndef _VAR_H_
#define _VAR_H_

#include "bool.h"

// API

bool                var_set(const char *name, double val);
bool                var_isset(const char *name);
bool                var_unset(const char *name);
double              var_get(const char *name); // 0.0 if not exists
void                var_clean(void);    // for array too!
bool                var_setarray(const char *name, int size);
int                 var_getarrsize(const char *name);
double             *var_getarr(const char *name);

#endif /* _VAR_H_ */
