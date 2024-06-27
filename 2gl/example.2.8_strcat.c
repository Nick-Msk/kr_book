#include <stdio.h>
#include <string.h>

char 				*locstrcat(char *s, int len, const char *orig);

int					main(int argc, const char *argv[]){

	if (argc < 3){
		fprintf(stderr, "Usage: %s str1 str2\n", *argv);
		return 1;
	}

	const char *s1 = argv[1];
	const char *s2 = argv[2];
	int 		len1 = strlen(s1), i = 0;
	int			lensum = len1 + strlen(s2) + 1;
	char        arr[lensum];	

	// copy
	while (*s1 != '\0')
		arr[i++] = *s1++;
	arr[i++] = '\0';

	printf("[%s]\n", locstrcat(arr, len1, s2));

	return 0;
}

char                *locstrcat(char *s, int slen, const char *orig){
	int		j = 0;
	
	if (slen == 0)
		while (s[slen++] == '\0')
			;
	while ( (s[slen++] = orig[j++]) != '\0')
		;
	return s;
}
