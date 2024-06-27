#include <stdio.h>


int			atoint(const char *s);


int			main(int argc, const char *argv[]){

	if (argc < 2)
		printf("Usage: %s intval\n", *argv);
	else {
		int 	val = atoint(argv[1]);
		printf("%s + 1 = %d\n", argv[1], val + 1);
	}
	return 0;
}

int         atoint(const char *s){
	
	int			res = 0;
	for (int i = 0; s[i] >= '0' && s[i] <= '9'; i++)
		res = res * 10 + (s[i] - '0');
	return res;
}


