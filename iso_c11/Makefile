CC=gcc #clang
CFLAGS= -I../utils/ -Wall -Wextra -std=gnu11 -g -O3
# -Wno-initializer-overrides -Wno-unused-function -Wno-format-zero-length
LDFLAGS=-
LDLIBS=

all:

malloc_test: src/malloc_test.c
	$(CC) $(CFLAGS) $^ -o out/$@_test

printf_n: printf_n.c
	$(CC) $(CFLAGS) $^ -o out/$@_test

round_up2: src/round_up2.c
	$(CC) $(CFLAGS) $^ -o out/$@_test

power: src/power.c
	$(CC) $(CFLAGS) $^ -o out/$@_test

t7.8.1: t7.8.1.c
	$(CC) $(CFLAGS) $^ -o $@_test

va_test: va_test.c ../log.c
	$(CC) $(CFLAGS) $^ -o $@_test

limit-test: limit-test.c
	$(CC) $(CFLAGS) $^ -o $@_test

gethostname_t: gethostname_t.c
	$(CC) $(CFLAGS) $^ -o $@_test

setjump_test: setjump_test.c
	$(CC) $(CFLAGS) $^ -o $@

assert_test: assert_test.c
	$(CC) $(CFLAGS) $^ -o $@

address_in_stack: address_in_stack.c
	$(CC) $(CFLAGS) $^ -o $@_test

strerror_test: strerror_test.c
	$(CC) $(CFLAGS) $^ -o $@

return_test: return_test.c
	$(CC) $(CFLAGS) $^ -o $@

setjump2_test: setjump2_test.c
	$(CC) $(CFLAGS) $^ -o $@

comparator_test: comparator_test.c ../log.c
	#$(CC) $(CFLAGS) $^ -o $@
	$(CC) $(CFLAGS) $^ -o $@

macro-cat-test: macro-cat-test.c
	$(CC) -E $^

timespec_get_test: timespec_get_test.c
	$(CC) $(CFLAGS) $^ -o $@

test-cmp-doc: test-cmp-doc.c
	$(CC) $(CFLAGS) $^ -o $@_test

# WO error fow now, for simplify
#error: error.o error.h log.c common.c	# can depends on log, not sure
#	$(CC) $(FLAGS) $^ -o $@_test

#error.o: error.c

.PHONY: clean cleanlog cleanall no_targets__ list
clean:
	-rm *.o
	-rm *_test
	-rm -r *.dSYM
	-rm *.gch
	find . -perm -u=x -type f -ls -maxdepth 1 -exec rm {} \;

cleanlog:
	-rm *.log

cleanall: clean cleanlog

no_targets__:
list:
	sh -c "$(MAKE) -p no_targets__ | awk -F':' '/^[a-zA-Z0-9][^\$$#\/\\t=]*:([^=]|$$)/ {split(\$$1,A,/ /);for(i in A)print A[i]}' | grep -v '__\$$' | sort"

