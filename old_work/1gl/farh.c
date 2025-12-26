#include <stdio.h>

static inline		float fahr_to_cels(float f){
	return (5.0 / 9.0) * (f - 32.0);
}

int			main(void){
	float	fahr, cel;
	int		lower = 0, upper = 300, step = 20;		// just init values
	
	// main cycle	
	fahr = lower;
	printf("Fahrengait Celcium\n");
	while (fahr <= upper){
		cel = fahr_to_cels(fahr);
		printf("%3.0f        %6.1f\n", fahr, cel);
		fahr += step;
	}
	return 0;
}
