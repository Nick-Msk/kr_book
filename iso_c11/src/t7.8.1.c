#include <inttypes.h>
#include <wchar.h>

int main(void){
	uintmax_t i = UINTMAX_MAX;    //this type always exists
	intmax_t im = INTMAX_MAX;
	wprintf(L"The largest integer value is %020"PRIxMAX "\n", i);
	wprintf(L" as llong: %lld, %lld\n", i, im);
	return 0;
}

