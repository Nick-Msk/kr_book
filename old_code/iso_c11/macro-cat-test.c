#define cat(x, y) x ## y

#define xcat(x, y) cat(x, y)

int		main(void){
	cat(1, 2);
	cat(1, cat(2, 3));
	xcat(cat(4, 5), 6);
	xcat(1, xcat(2, 3));
	xcat(1, cat(2, 3));
}
