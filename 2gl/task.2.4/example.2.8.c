#include <stdio.h>

const char 			*squeeze(char *s, char c);


int					main(int argc, char *argv[]){
	
	if (argc < 3){
		fprintf(stderr, "Usage: %s symbol string\n", *argv);
		return 1;
	}

	const char *sym = argv[1];
	if (sym[1] != '\0'){
		fprintf(stderr, "Must be a single symbol [%s]\n", sym);
		return 2;
	}

	printf("[%s]\n", squeeze(argv[2], sym[0]));

	return 0;
}

const char 			*squeeze(char *s, char c){
	int		i, j;
	for (i = j = 0; s[i] != '\0'; i++)
		if (s[i] != c)
			s[j++] = s[i];
	
	s[j] = '\0';
	return s;
}
