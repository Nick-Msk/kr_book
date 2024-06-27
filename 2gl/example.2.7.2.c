#include <stdio.h>
#include <stdlib.h>

unsigned long	rand_next;
int				rand(void);
void			srand(unsigned seed);

int			main(int argc, const char *argv[]){
	unsigned 	seed = 1001;
	if (argc > 1)
		seed = abs(atoi(argv[1]));

	srand(seed);
	
	for (int i = 0; i < 10; i++)
		printf("%d\n", rand());
		
	return 0;
}

void        srand(unsigned seed){
	rand_next = seed;
}

int			rand(void){
	rand_next = rand_next * 1103515245 + 12345;
	return (unsigned) (rand_next/65536) % 32768;
}


