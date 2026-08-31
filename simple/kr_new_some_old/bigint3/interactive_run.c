#include <stdio.h>
#include <stdlib.h>
#include "bigint.h"


int main(int argc, char **argv){
	if(argc<4){	
		fprintf(stderr, "usage: %s int1 int2 ...\n", argv[0]);
		return 1;
	}
	int SZ=100, BSZ=1000, cnt=0;			
	Bigint *v[SZ];
	char buf1[BSZ], buf2[BSZ], buf3[BSZ];
	argv++;
	while(argv && cnt<SZ && cnt < argc-1)
		v[cnt++]=bi_create(*argv++);
	long lmax, ll;
	bi_getlong(v[3], &ll);  bi_getlong(v[2], &lmax);
	printf("max=%ld inc=%ld\n", lmax, ll);
	//bi_dump_all(true);
	for(int i=0; i<lmax; i++){
		bi_to_str(v[1], buf2, BSZ);
		printf("[%d]:%s / %s = ", i,  bi_to_str(v[0], buf1, BSZ), buf2);
		Bigint *vv=bi_div(v[0], v[1]);
		printf("%s\n", bi_to_str(vv, buf3, BSZ));
		v[0]=bi_add(v[0], bi_create_l(ll));   //bi_inc(v[0]);
	}

	//bi_dump_all(false);
	
/*	Bigint *v6=bi_sub(v[0], v[1]);
	bi_dump(v6);
	return 0; 
	
	Bigint *v3=bi_add(v[0], v[1]);

	bi_dump_all(false);

	//bi_se_fastpow(v[2], 35);
	
	//bi_dump(v[1]);

	Bigint *v4=bi_mul(v[0], v[1]);

	//bi_dump_all(false);
	
	printf("------------------------\n");
	bi_dump(v3);

	bi_dump(v4);
 	
	Bigint *v5=bi_div(v[0], v[1]);
	printf("div:\n");
	bi_dump(v5);

	
	
	Bigint *v3=bint_add(v1, v2);

	printf("%s+%s=%s\n", bint_getstr(v1), bint_getstr(v2), bint_getstr(v3));
	
	Bigint *v4=bint_mul(v1, v2);
	
	//bigint_str(v1, buf1, 1000); bigint_str(v2, buf2, 1000); bigint_str(v3, buf3, 1000);
	printf("%s*%s=%s\n", bint_getstr(v1), bint_getstr(v2), bint_getstr(v4)	);    */
	bi_clear();	// crear all 
//	bi_dump_all(false);
	return 0;
}

