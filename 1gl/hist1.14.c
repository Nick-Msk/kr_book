#include <stdio.h>

#define			MAX_LENGTH 		100
#define			NOPRINTEMPTY 	0
#define			PRINTEMPTY		1

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

int			print_array(int cnts[], int size, int flag){
	int		sz = 0;
	for (int i = 0; i < size; i++){
		if (cnts[i] || flag)
			sz += printf("len %d = %d\n", i, cnts[i]);
	}
	return sz;
}

int			main(void){
	int			cnts[MAX_LENGTH];
	for (int i = 0; i < MAX_LENGTH; i++)
        cnts[i] = 0;
	
	int total = 
		calculate_word_length(cnts, MAX_LENGTH);
	print_array(cnts, MAX_LENGTH, NOPRINTEMPTY);
	printf("Total words = %d\n", total);
		
	return 0;
}
