#include <stdio.h>
#include <stdlib.h>

#define			MAX_LENGTH 		100
#define			NOPRINTEMPTY 	0
#define			PRINTEMPTY		1
#define			DEFLINESIZE		200
#define			MINLINESIZE		50

int			calculate_word_length(int cnts[], int size){
	int         len = 0, total = 0, c;

    for (int i = 0; i < MAX_LENGTH; i++)
        cnts[i] = 0;

    while ((c = getchar()) != EOF){
        if (c == ' ' || c == '\t' || c == '\n'){
            if (len > 0)        // prev word here
                cnts[len]++, total++; 
            len = 0;
        } else
			len++;
    }   
	// last word (if EOF just after)
	if (len > 0)
		cnts[len]++, total++;
	return total;
}

int			print_vert_array(int cnts[], int size, int linesize){
	int		sz = 0, pos = 0, val = 0, i;
	//								LINESIZE
	// 1	2	5	15		16		...
	// 25	4	3	3		4		...
	for (i = 0; i < size && pos < linesize; i++){
		if (cnts[i]){
			val = printf("%d\t", i);
			sz += val;
			pos += val;
			//fprintf(stderr, "%d %d %d\n", i, pos, val);
		}	
	}
	if (i < size)
		sz += printf("....\t"); 
	putchar('\n');
	for (int j = 0; j < i; j++){
		if (cnts[j])
			sz += printf("%d\t", cnts[j]);
	}
	putchar('\n');
	return sz;
}

int			print_array(int cnts[], int size, int flag){
	int		sz = 0;
	for (int i = 0; i < size; i++){
		if (cnts[i] || flag)
			sz += printf("len %d = %d\n", i, cnts[i]);
	}
	return sz;
}

int			main(int argc, char **argv){
	int			cnts[MAX_LENGTH];
	int			linesize = DEFLINESIZE;	// default

	// check if linesize is provided
	if (argc > 1)
		linesize = atoi(argv[1]);

	if (linesize < MINLINESIZE)
		linesize = MINLINESIZE;

	for (int i = 0; i < MAX_LENGTH; i++)
        cnts[i] = 0;
	
	int total = 
		calculate_word_length(cnts, MAX_LENGTH);

	print_vert_array(cnts, MAX_LENGTH, DEFLINESIZE);
	printf("Total words = %d\n", total);
		
	return 0;
}
