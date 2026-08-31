#include <stdio.h>

#define 	LOWER		0
#define		UPPER		300
#define 	STEP		20

float			fahr_to_cels(int fahr){
	return (5.0 / 9.0) * (fahr - 32);
}

int			main(void){
	for (int fahr = UPPER; fahr >= LOWER; fahr -= STEP)
		printf("%3d %6.1f\n", fahr, fahr_to_cels(fahr));

	return 0;
} 
