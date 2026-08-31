#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include "bigint.h"
#include <log.h>
#include <error.h>

//------------------------------ variables -----------------------------
static Bigint 				**g_alloc   = 0;
static int					g_alloc_sz  = 0;
static const int 			G_ALLOC_INC = 10;
static const int 			BASE 		= 10;

// util
static inline 				int max(int a, int b) {
	return a > b ? a : b;
}

static inline               bool eng_invalid(const Bigint *v);

static inline 				bool eng_check(const Bigint *v){
	return v == 0 || v->val == 0 || eng_invalid(v);
}

static void 	*mem_realloc(void *s, size_t sz, int *cnt, int inc);
static int 		mem_find_free(void);
// engine
// constructors
static Bigint  				*eng_alloc(const char *str, int len, bool sgn, bool convert);
// operators
static Bigint  				*eng_add(Bigint *, Bigint *);	// чистое сложение
static Bigint				*eng_sub(Bigint *, Bigint *);	// чистое вычитание, v1 >= v2
static Bigint				*end_mul(Bigint *, Bigint *);   // чистое беззнаковое умножение
static Bigint				*eng_div(Bigint *, Bigint *, Bigint **);	// v2 != 0,  знаки не учитываются

static inline 				bool eng_sign(const Bigint *v){
	return (v->flags & BI_F_SIGN)>0;
}

static inline 				bool eng_invalid(const Bigint *v){
	return (v->flags & BI_F_INVALID)>0;
}

// self-operators

// engine
static inline Bigint 		*eng_se_setinvalid(Bigint *v){
	v->flags |= BI_F_INVALID;
	return v;
}

// engine
static inline Bigint   		*eng_se_setnegative(Bigint *v){
	v->flags |= BI_F_SIGN;
	return v;
}

//engine
static inline Bigint   		*eng_se_setpositive(Bigint *v){
	v->flags &= ~BI_F_SIGN;
	return v;
}

//engine
static inline Bigint    	*eng_se_setsign(Bigint *v, bool sign){
    if(sign)
        return eng_se_setnegative(v);
    else
        return eng_se_setpositive(v);
}


// self negative
static inline Bigint   		*eng_se_negative(Bigint *v){
    if(!eng_sign(v))
        return eng_se_setnegative(v);
    else
        return eng_se_setpositive(v);
}

// interface
Bigint      				*bi_se_negative(Bigint *v){
	if(!eng_check(v))
		return eng_se_negative(v);
	else
		return v;
}

// engine, reallocate
static Bigint				*eng_se_reallocate(Bigint *v, int inc){
	char *nv = mem_realloc(v->val, sizeof(char), &v->sz, inc);
	if(nv == 0)
		return 0;
	v->val = nv;	// reassign
	return v;
}

// engine, divider > 0, nochecking
static Bigint 				*eng_se_fastdiv(Bigint *v, int power){
	for(int i = 0; i < v->len-power; i++)
		v->val[i] = v->val[i + power];
	for(int i=v -> len-power; i < v->len; i++)	// вообще-то это не надо, на всякий случай
		v-> val[i] = 0;
	v->len -= power;
	return v;
}

//engine, power > 0
static  Bigint				*eng_se_shift(Bigint *v, int power){
	if(v->len + power > v->sz)
		return 0;	// actually don't need it... but just in case
	// memmove must be here... but now simple cycle
    for(int i = v->len - 1; i >= 0; i--) // todo: возможно стоит вынести в отдельную функцию eng_se_shift
        v->val[i + power] = v->val[i];
    // fill with 0 the rest
    for(int i = 0; i < power; i++)
       v->val[i] = 0;
    v->len += power;
	return v;
}

// engine,  fast selt mult, se - self marker , power > 0
static 	Bigint				*eng_se_fastmul(Bigint *v, int power){
	// printf("deb fastpow: v=%s, pow=%d\n", bint_getstr(v), power);
	if(v->len + power>v->sz)
		if(!eng_se_reallocate(v, power + 10))	// increase size of val (sz)
			return 0;
	return eng_se_shift(v, power);
}

// interface
Bigint                      *bi_se_fastdiv(Bigint *v, int power){
	if(eng_check(v))
		return v;
	if(power <= 0)
		return v;
	return eng_se_fastdiv(v, power);
};

// interface
Bigint          			*bi_se_fastmul(Bigint *v, int power){
	if(eng_check(v))
		return 0;
	if(power<=0)
		return v;
	return eng_se_fastmul(v, power);
}

// comparators

// very fast simple 1 detector
static inline 				bool  eng_is_one(const Bigint *v){
	return v->len == 1 && *v->val == 1;
}

// very fast and simple zero detector
static inline 				bool 	eng_is_zero(const Bigint *v){
	return v->len == 1 && *v->val == 0;
}

// interface
bool 						bi_is_zero(Bigint *v){
	if(v && v->val)
		return eng_is_zero(v);
	else
		return false;	// TODO: use userraiseint here!!!
}

static 						int eng_cmp(Bigint *, Bigint *);

static inline Bigint 		*eng_max(Bigint *v1, Bigint *v2){
	if(eng_cmp(v1, v2)>=0)
		return v1;
	else
		return v2;
}

static inline Bigint 		*eng_min(Bigint *v1, Bigint *v2){
	if(eng_cmp(v1, v2)>=0)
		return v2;
	else
		return v1;
}

// interface
int		        			bi_cmp(Bigint *v1, Bigint *v2){
	if(v1 == 0 || v2 == 0 || v1->val == 0 || v2->val == 0)
		return 0;
	// todo: обработать знаки!!!
	bool sign;
	if( (sign=eng_sign(v1)) - eng_sign(v2) == 0)
		return sign ? -eng_cmp(v1, v2) : eng_cmp(v1,v2);
	else
		return -(eng_sign(v1) - eng_sign(v2));
}

// engine , v1 and v2 must be ok ( !=0 and v->val != 0)
static int 					eng_cmp(Bigint *v1, Bigint *v2){
	if(v1->len == v2->len) {
		for(int i = v2->len - 1; i >= 0; i--)
			if(v1->val[i] != v2->val[i])
				return v1->val[i] - v2->val[i];
		return 0;	// if not found diff then equal
	}
	else
		return v1->len - v2->len;
}

// memory functions (can be moved to another file )
static void					*mem_realloc(void *s, size_t sz, int *cnt, int inc){
	if(inc < 0 || cnt == 0 ||  *cnt < 0)
		return 0;
	if(inc == 0)
		return s;
	void *t;
	if( (t = malloc(sz*(*cnt + inc)) ) ==0)
		return 0;		// can't allocate
	memcpy(t, s, sz* *cnt);
	memset((char *)t + sz* *cnt, 0, sz * inc);
	*cnt += inc;	// adjust size
	free(s);
	return t;
}

static int					mem_find_free(void){
	for(int i = 0; i < g_alloc_sz; i++)
		if(g_alloc[i] == 0 || g_alloc[i]->val == 0)
			return i;
	return -1;
}

// ----------------------------------------------------------------------------------------

// -----------------------------------------------------------------------------------------

//  destructors
int         				bi_free(Bigint *v){
	if(v->val){
		free(v->val);
		v->val = 0;
	}
	//v->flags |= BI_F_FREE;  no more need, use val
	return 0;
}

int							bi_clear(void){
	for(int	i = 0; i < g_alloc_sz; i++)
		if(g_alloc[i])
			bi_free(g_alloc[i]);
	return 0;
}

// contructors

// dummy creator vithout val
static inline  Bigint   	*eng_create_pos(void){
	Bigint *v;
	if( (v = malloc(sizeof(Bigint))) == 0)
		return 0;
	v->flags = v->len = v->sz = 0;
	v->val = 0;
	return v;
}

// str - set of 0..9, NOT '0' .. '9', if str <null> use len
static Bigint 				*eng_create_val(Bigint *v, const char *str, int len, bool sgn, bool convert){
	if(v == 0)
		return 0;
	if( (v->val = malloc(len * sizeof(char))) == 0)
		return 0;	// failed
	if(str)
		if(convert)
    		for(int i = 0; i < len; i++)
	    		v->val[len - 1 - i] = str[i] - '0';    // convert to values
		else 	// direct copy, we can use memcpy
			for(int i = 0; i < len; i++)
				v->val[i] = str[i];	// without changing
	else
		memset(v->val, 0, len);	// INIT WITH 0
	v->sz = v->len = len;
	return eng_se_setsign(v, sgn);
}

// internal, basic constructor, not verify params
static Bigint				*eng_alloc(const char *str, int len, bool sgn, bool convert){
	int pt=mem_find_free();
	if(pt < 0) {
		Bigint **nt=0;
		if( (nt = mem_realloc(g_alloc, sizeof(Bigint *), &g_alloc_sz, G_ALLOC_INC))==0)
			return 0; // can't allocate
		else
			g_alloc = nt;
		if(( pt = mem_find_free()) <0)	// try one more time
			return 0;
	}
	if(g_alloc[pt] == 0)  // 2 варианта свободного, с созданным элементом и совсем пустой
		g_alloc[pt] = eng_create_pos();
	return eng_create_val(g_alloc[pt], str, len, sgn, convert);
}

// interface
Bigint						*bi_move(Bigint *dest, Bigint *src){
	if(eng_check(src))
		return src;
	bi_free(dest);	// correct, because free check dest
	dest->val = src->val;
	src->val = 0;	// just like free, but without free
	dest->len = src->len;
	dest->sz = src->sz;
	return dest;
}

// interface
Bigint						*bi_copy(Bigint *dest, Bigint *src){
	if(eng_check(src))
		return src;
	Bigint *tmp = bi_clone(src);
	return bi_move(dest, tmp);
}

//interface
Bigint                      *bi_create_10(int power){ // 10^power
	if(power<0)
		return 0;
	Bigint *v = eng_alloc(0, power + 1, false, false);
	v->val[power] = 1;
	return v;
}

// interface
Bigint      				*bi_create(const char *s){
	if(s==0)
		return 0;
	while(isspace(*s))	// skip spaces
		s++;
	bool sign = false;
	if(*s == '-')
		sign = true;
	if(*s == '+' || * s== '-')
		s++;
	if(*s == '\0')
		return 0;	// "+" or "-" is not correct
	while(*s == '0')	// skip zeros
		s++;
	if(*s == '\0')
		s--;
	int len = 0;
	while(s[len])
		if(!isdigit(s[len++])) // check for only digits and find length
			return 0;
	return eng_alloc(s, len, sign, true);
}

// interface
Bigint      				*bi_clone(Bigint *v){
	if(eng_check(v)) // free position
		return v;
	return eng_alloc(v->val, v->len, eng_sign(v), false);
}

// printers

bool						bi_getlong(Bigint *v, long *l){
	if(eng_check(v) || l==0 || v->len > 18) // ??? 18
		return false;
    long lv = 0;
	for(int i = v->len - 1; i >= 0; i--)
		lv = lv * 10 + v->val[i];
	if(eng_sign(v))
		lv = -lv;
	*l = lv;
	return true;
}

// main user printer  TODO: bigint - iterator is STRICTLY required
const char  				*bi_to_str(Bigint *v, char *buf, int sz){
	if(eng_check(v) || buf == 0 || sz <= 1)
		return 0;
	logenter("%p, size of buf %d", v, sz);

	// now v is ok, sz >=2
	int pt = 0;
	bool sign=eng_sign(v);
	logauto(sign);

	if(sign)
		buf[pt++] = '-';
	if(pt >= sz-1)	// because finish '\0'
		return 0;

	logmsg("sz = %d, loen = %d", v->sz, v->len);


	lognumbers(v->val, v->sz);	// TODO: create this, have to put string with sz length, with \n, \t, \0
	while(pt < sz - 1 && pt < v->len + sign){
		logmsg("pt = %d, v->len = %d, res = %d", pt, v->len, v->len + 1 - pt + sign);
		logmsg("[%c]", v->val[v->len - 1 - pt + sign] + '0');

		buf[pt] = v->val[v->len - 1 - pt + sign] + '0';
		pt++;
	}

	buf[pt] = '\0';
	return buf;
}

// technical , dump view
int     					bi_fdump(FILE *restrict out, const Bigint *restrict v){
	if(v == 0)
		return 0;
	if(v->val == 0)
		return fprintf(out, "(freed)\n");
	int cnt = fprintf(out, "len=%d sz=%d sign=%s invalid=%s\n",
		v->len, v->sz, bool_str(eng_sign(v)), bool_str(eng_invalid(v)) );
	if(eng_is_zero(v))
		fputc('0', out);
	else {
		if(eng_sign(v))
			fputc('-', out), cnt++;
		for(int i = 0; i < v->len; i++)
			fputc(v->val[v->len - 1 - i] + '0', out);
	}
	fputc('\n', out);
	return cnt + v->len + 1;
}

// technical, total dump
int     					bi_fdump_all(FILE *out, bool print_null){
	int cnt=0;
	for(int i = 0; i < g_alloc_sz; i++)
		if(g_alloc[i]){
			cnt += fprintf(out, "[%d]: ", i);
			cnt += bi_fdump(out, g_alloc[i]);
		} else if(print_null)
			cnt += fprintf(out, "(null)\n");
	return cnt;
}

// operators

// technical, len nomalization  0000000567 -> 567
static inline 				int	eng_shrink_len(Bigint *v) {
	int nlen = v->len;
	while(nlen > 1 && v->val[nlen - 1]==0)
        nlen--;
    return nlen;
}

// engine, summ without care of sign
Bigint 						*eng_add(Bigint *v1, Bigint *v2){
	int nlen=max(v1->len, v2->len) + 1;
	int res=0;
	Bigint *v = eng_alloc(0, nlen, false, false);
	if(v == 0)
		return 0;
	for(int i = 0; i < nlen; i++){
		int r1 = 0, r2 = 0;
		if(i < v1->len)
			r1 = v1->val[i];
		if(i < v2->len)
			r2 = v2->val[i];	// остаток с предыдущего уровня , строго < BASE
		v->val[i] = (res += r1 + r2) % BASE;
		//printf("deb: v[%d]=%d res=%d r1=%d r2=%d \n", i, v.val[i], res, r1, r2);
		res /= BASE; 	// остаток на следующий уровень
	}
	/*if(v->val[nlen-1]==0)
		v->len--;	// length correction */
	v->len=eng_shrink_len(v);
	return v;
}

// sybstr in ring BASE
static inline 				int ring_base_minus(int r1, int r2, int pr){
	return (BASE + r1 - r2 - pr)%BASE;
}

// engine, v1 >= v2
Bigint 						*eng_sub(Bigint *v1, Bigint *v2){
	int nlen = v1->len, res=0;
	Bigint *v = eng_alloc(0, nlen, false, false);
	if(v == 0)
		return 0; 	// can't allocate
	for(int i = 0; i < nlen; i++){
		int r1 = 0, r2 = 0;
		if(i < v1->len)
			r1 = v1->val[i];
		if(i < v2->len)
			r2 = v2->val[i];
		v->val[i] = ring_base_minus(r1, r2, res);
		res = (r2 + res)>r1?1:0;	//  1 if r2 + res more than r1 ,  res is calc on prev level
	}
	// length correction here , можно вынести в отдельную функцию !!!
	/*while(nlen>1 && v->val[nlen-1]==0)
		nlen--; */
	v->len = eng_shrink_len(v); //nlen;
	return v;
}

// inetrface , v1 + <null> = v1
Bigint						*bi_add(Bigint *v1, Bigint *v2){
	if(eng_check(v1))
        return v1;
	if(eng_check(v2))
		return v2;
	if(eng_is_zero(v1))
		return bi_clone(v2);
	if(eng_is_zero(v2))
		return bi_clone(v1);
    bool sgn1 = eng_sign(v1), sgn2 = eng_sign(v2);
	if( (sgn1 && !sgn2) || (!sgn1 && sgn2) ) {	// signs are diff
		Bigint *vm = eng_max(v1, v2), *vi = eng_min(v1, v2);
		Bigint *v = eng_sub(vm, vi);
		return eng_se_setsign(v, eng_sign(vm));
	} else { // the same sign
		Bigint *v = eng_add(v1, v2);
		return eng_se_setsign(v, sgn1);
	}
}

// interface, v1 - v2
Bigint 						*bi_sub(Bigint *v1, Bigint *v2){
	if(eng_check(v1))
	     return v1;
	if(eng_check(v2))
		return v2;
	if(eng_is_zero(v1))
		return bi_se_negative(bi_clone(v2)); // <null> - v2 = -v2
	if(eng_is_zero(v2))
		return bi_clone(v1);	// v1 - <null> = v1
	return bi_add(v1, eng_se_negative(v2));
}

// engine , r is 1..9
Bigint						*eng_mul_1(Bigint *v, int rv){
	if(rv == 1)
		return bi_clone(v);
	int res=0, new_sz = v->len + 1;
	Bigint 	*r = eng_alloc(0, new_sz, eng_sign(v), false);
	// the same circle as for eng_add and eng_sub
	for(int i = 0; i < new_sz; i++){
		r->val[i] = (res += rv* v->val[i]) %BASE;
		res /= BASE;
	}
	r->len = eng_shrink_len(r);
	return r;		// sign and sz are already setted up by eng_alloc
}

//engine
Bigint						*eng_mul(Bigint *v1, Bigint *v2){
	Bigint 	*vmin = eng_min(v1, v2), *vmax = eng_max(v1, v2);
	int mlen = vmin->len;
	Bigint	*v = bi_create_zero();
	for(int i = 0; i < mlen; i++)
		if(vmin->val[i]) {
			v=eng_add(v, eng_se_fastmul(eng_mul_1(vmax, vmin->val[i]), i));
			bi_dump(v);
		}
	return v;
}

// interface
Bigint						*bi_mul(Bigint *v1, Bigint *v2){
	if(eng_check(v1))
		return v1;
	if(eng_check(v2))
		return v2;
	if(eng_is_zero(v1) || eng_is_zero(v2))
		return bi_create_zero();
	Bigint *v;
	if(eng_is_one(v1))
		v = bi_clone(v2);
	else if(eng_is_one(v2))
		v = bi_clone(v1);
	else v = eng_mul(v1, v2);
	return eng_se_setsign(v, eng_sign(v1) ^ eng_sign(v2));
}

// engine , v1 > v2 strictly
static Bigint				*eng_div(Bigint *v1, Bigint *v2, Bigint **vres){
	Bigint  *res = bi_create_zero();	// temp
	Bigint  *vd = bi_clone(v1);
	do {
		Bigint  *v = bi_clone(v2);
		int power = 0, r = 0;
		while(eng_cmp(vd, v) >= 0)
			eng_se_fastmul(v, 1), power++;  // это, конечно, ожно сделать эффективнее
		if(power)	// если было что-то, то отмена
			eng_se_fastdiv(v, 1), power--;

		while(eng_cmp(vd, v) >= 0)
			vd=eng_sub(vd, v), r++;

		//////
		// assert(r/10);	// r must be <10
		//printf("r=%d power=%d\n", r, power);
		if(r)
			res=eng_add(res, eng_se_fastmul(bi_create_l(r), power));

	} while(eng_cmp(vd, v2)>=0);  // while dv > v2
	if(vres)
		*vres = vd;	// v1%v2
	return res;
}

// interface
Bigint						*bi_div(Bigint *v1, Bigint *v2) {
	if(eng_check(v1))
        return v1;
	if(eng_check(v2))
		return v2;
	if(eng_is_zero(v2))
		return eng_se_setinvalid(bi_create_zero());
	int comp;
	if(eng_is_zero(v1) || (comp=eng_cmp(v1, v2)) < 0)
		return bi_create_zero();
	Bigint *v;
	if(comp == 0)	// equal
		v = bi_create_one();
	else if(eng_is_one(v2))
		v = bi_clone(v1);	// clone is necessary
	else
		v = eng_div(v1, v2, 0);
	return eng_se_setsign(v, eng_sign(v1) ^ eng_sign(v2));
}

// interface
Bigint						*bi_mod(Bigint *v1, Bigint *v2) {
	if(eng_check(v1))
        return v1;
	if(eng_check(v2))
		return v2;
    if(eng_is_zero(v2))
       return eng_se_setinvalid(bi_create_zero());
    int comp;
    if(eng_is_zero(v1) || eng_is_one(v2) || (comp = eng_cmp(v1, v2))==0)
        return bi_create_zero();
	Bigint *v = bi_create_zero();
	eng_div(v1, v2, &v);
	return v;
}

// interface   , v+1
Bigint                      *bi_inc(Bigint *v){
	if(eng_check(v))
		return v;
	if(eng_sign(v))
		return eng_sub(v, bi_create_one());
	else
		return eng_add(v, bi_create_one());
}

// interface, v-1
Bigint						*bi_dec(Bigint *v){
	if(eng_check(v))
		return v;
	if(eng_sign(v))
		return eng_add(v, bi_create_one());
	else
		return eng_sub(v, bi_create_one());
}

// -------------------------------Testing --------------------------
#ifdef BIGINTTESTING

#include "testing.h"
#include <checker.h>

//types for testing

// ------------------------- TEST 1 ---------------------------------

// Basic create from string/free test (private test)
static TestStatus
tf1(void)
{
    logenter("%s", __func__);

	Bigint *b = bi_create("12345");		// TODO: think, to avoid Bigint *, constructor have to return Bigint

	if (!b)
		return logacterr(err_clean(true), TEST_FAILED, "Unable to create Bigint");


	bi_fdump(logfile, b);

	// try via invariant
	//inv();
	if (bi_isnegative(b))
		return logacterr(bi_free(b), TEST_FAILED, "Must be positive");		// TODO: not sure about err_clean

	logmsg("print bigint into string");
	char buf[100];	// more than enough

	bi_to_str(b, buf, sizeof buf);

	if (strcmp(buf, "12345") != 0)
		return logacterr(bi_free(b), TEST_FAILED, "Value must be 12345, but it is [%s]", buf);

	logmsg("print bigint into long");

	long tmp;

	if (!bi_getlong(b, &tmp))
		return logacterr(bi_free(b), TEST_FAILED, "Failed while converting into long");

	if (tmp != 12345)
		return logacterr(bi_free(b), TEST_FAILED, "Value must be 12345, but it is %ld", tmp);

	err_clean(true);
	bi_free(b);

    return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// Basic create from long and clone (private test)
static TestStatus
tf2(void)
{
	logenter("%s", __func__);

	Bigint	*b;
	long 	val = -45678;

	test_sub("create from val");
	if ( (b = bi_create_l(val)) == 0)
		return logerr(TEST_FAILED, "Unable to create bigint from long");

	bi_fdump(logfile, b);

	if (!bi_isnegative(b))
        return logacterr(bi_free(b), TEST_FAILED, "Must be not positive");

	test_sub("print bigint into string");

	char buf[100];  // more than enough
	bi_to_str(b, buf, sizeof buf);

	if (strcmp(buf, "-45678") != 0)
        return logacterr(bi_free(b), TEST_FAILED, "Value must be %ld, but it is [%s]", val, buf);

	test_sub("Cloning");

	err_clean(true);
	bi_free(b);

	return logret(TEST_PASSED, "done"); // TEST_FAILED
}

// ------------------------------------------------------------------
int
main(int argc, char *argv[])
{
    LOG(const char *logfilename = "bigint.log");

    if (argc > 1)
        logfilename = argv[1];

    loginit(logfilename, false, 0, "Starting");

        testenginestd(
            testnew(.f2 = tf1, .num = 1, .name = "Basic create from string/free test", .desc = "( and from long too). Private."             , .mandatory=true)
		  , testnew(.f2 = tf2, .num = 2, .name = "Basic create from long and clone"  , .desc = "Private."              						, .mandatory=true)
		);

	logclose("end...");
    return 0;
}


#endif /* BIGINTTESTING */


