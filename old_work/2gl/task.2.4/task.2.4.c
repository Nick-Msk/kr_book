#include <stdio.h>

const char 			*squeezestr(char *s, const char *syms);


int					main(int argc, char *argv[]){
	
	if (argc < 3){
		fprintf(stderr, "Usage: %s syms target_string\n", *argv);
		return 1;
	}

	const char 	*syms = argv[1];
	char 		*str = argv[2];

	printf("[%s]\n", squeezestr(str, syms));

	return 0;
}

const char 			*squeezestr(char *s, const char *syms){
	
	int		i, j;
	for (i = j = 0; s[i] != '\0'; i++){
		int		found = 0;
		for (int k = 0; found == 0 && syms[k] != '\0'; k++)
			if (s[i] == syms[k])
				found = 1;
		if (!found)
			s[j++] = s[i];
	
	}
	s[j] = '\0';
	return s;
}
