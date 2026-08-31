#include <math.h>
#include <stdio.h>

int main(){
	double val = M_PI/2.0;
	val = tan(val);

	printf("%.10e\n", val);
}
