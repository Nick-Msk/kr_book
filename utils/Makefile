CC=gcc #clang
CFLAGS= -I./ -Wall -Wextra -std=gnu11 -g -O0 -Wno-initializer-overrides -Wno-gnu-folding-constant 
# -Wno-initializer-overrides -Wno-unused-function -Wno-format-zero-length
LDFLAGS=
LDLIBS=

all:

check: check.c log.o test.o error.o
	$(CC) $(CFLAGS) -DCHECKERTESTING $^ -o test/$@_test

check.o: check.c log.c
	$(CC) $(CFLAGS) $^ -c
	@mv $@ lib/

error: error.c log.o common.o test.o
	$(CC) $(CFLAGS) -DERRORTESTING $^ -o test/$@_test

error.o: error.c log.o common.o
	$(CC) $(CFLAGS)  $^ -c
	@mv $@ lib/

log: log.c test.o error.o
	$(CC) $(CFLAGS) -DLOGTESTING $^ -o test/$@_test

log.o:	log.c
	$(CC) $(CFLAGS) $^ -c
	@mv $@ lib/

common.o: common.c
	$(CC) $(CFLAGS) $^ -c
	@mv $@ lib

test: test.c error.o log.o
	$(CC) $(CFLAGS) -DTESTTESTING $^ -o test/$@_test


metric: metric.c log.c test.c
	$(CC) $(CFLAGS) -DCHECKERTESTING $^ -o test/$@_test
	@mv log.o test.o lib/

metric.o: metric.c
	$(CC) $(CFLAGS) $^ -c
	@mv $@ lib/

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

