#include <stdio.h>
#include <ctype.h>

int			htoi(const char *s);

int			main(int argc, const char *argv[]){
	
	if (argc < 2){
		fprintf(stderr, "Usage: %s hex_val\n", *argv);
		return 1;
	}

	int		val = htoi(argv[1]);

	printf("%s = %d\n", argv[1],  val);
	
	return 0;
}

static inline int 	is_hex(char c){
	return isdigit(c) || (tolower(c) >= 'a' && tolower(c) <= 'f' );
}

static inline int 	hex_to_digit(char c){

	int		res;
	c = tolower(c);
	if (isdigit(c))
		res = c - '0';
	else if (c >= 'a' && c <= 'f')
		res = c - 'a' + 10;
	else
		res = 0;
	//fprintf(stderr, "c = %c, res = %d\n", c, res);
	return res;
}

int         		htoi(const char *s){

	int			res = 0, i = 0;
	while (isspace(s[i]))
		i++;
	if (s[i++] != '0' || tolower(s[i++]) != 'x')		// not a HEX
		return 0;	
	
	while (is_hex(s[i]))
		res = res * 16 + hex_to_digit(s[i++]); 
	
	return res;
}



