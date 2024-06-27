#include <assert.h>
#include <stdio.h>

#define myassert(expr)\
	(expr) ? printf("OK: %s\n", #expr) : printf(" FAILED %s\n", #expr)

int		main(void){
	myassert(2 + 5);
	myassert(3 - 3);
	assert(1 - 1);
}
