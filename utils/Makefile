CC=gcc #clang
CFLAGS= -I./ -Wall -Wextra -std=gnu11 -g -O0 -Wno-initializer-overrides -Wno-gnu-folding-constant 
# -Wno-initializer-overrides -Wno-unused-function -Wno-format-zero-length
LDFLAGS=
LDLIBS=

all:

checker: src/checker.c log.o test.o error.o
	$(CC) $(CFLAGS) -DCHECKERTESTING $^ -o test/$@_test

checker.o: src/checker.c
	$(CC) $(CFLAGS) $^ -c
	@mv $@ lib/

error: src/error.c log.o common.o test.o
	$(CC) $(CFLAGS) -DERRORTESTING $^ -o test/$@_test

error.o: src/error.c
	$(CC) $(CFLAGS)  $^ -c
	@mv $@ lib/

log: src/log.c test.o error.o
	$(CC) $(CFLAGS) -DLOGTESTING $^ -o test/$@_test

log.o:	src/log.c
	$(CC) $(CFLAGS) $^ -c
	@mv $@ lib/

common.o: src/common.c
	$(CC) $(CFLAGS) $^ -c
	@mv $@ lib

test.o:	src/test.c
	$(CC) $(CFLAGS) $^ -c
	@mv $@ lib/

test: src/test.c lib/error.o lib/log.o
	$(CC) $(CFLAGS) -DTESTTESTING $^ -o test/$@_test


metric: src/metric.c log.o test.o
	$(CC) $(CFLAGS) -DCHECKERTESTING $^ -o test/$@_test
	@mv log.o test.o lib/

metric.o: src/metric.c
	$(CC) $(CFLAGS) $^ -c
	@mv $@ lib/

array.o: src/array.c
	$(CC) $(CFLAGS) $^ -c
	@mv $@ lib/

array: src/array.c lib/log.o lib/test.o lib/error.o lib/checker.o
	$(CC) $(CFLAGS) -DARRAYTESTING $^ -o test/$@_test

.PHONY: clean cleanlog cleanall no_targets__ list
clean:
	@rm *.o
	@rm *_test
	@rm -r *.dSYM
	@rm *.gch
	@rm test/*
	@rm lib/*
	find . -perm -u=x -type f -ls -maxdepth 1 -exec rm {} \;

cleanlog:
	-rm *.log

cleanall: clean cleanlog

no_targets__:
list:
	sh -c "$(MAKE) -p no_targets__ | awk -F':' '/^[a-zA-Z0-9][^\$$#\/\\t=]*:([^=]|$$)/ {split(\$$1,A,/ /);for(i in A)print A[i]}' | grep -v '__\$$' | sort"

