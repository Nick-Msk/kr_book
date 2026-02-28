CC=gcc #clang
CFLAGS= -I./ -Wall -Wextra -std=gnu11 -g -O0 -Wno-initializer-overrides -Wno-gnu-folding-constant 
# -Wno-initializer-overrides -Wno-unused-function -Wno-format-zero-length
LDFLAGS=
LDLIBS=

all:

checker: src/checker.c lib/log.o lib/test.o lib/error.o
	$(CC) $(CFLAGS) -DCHECKERTESTING $^ -o test/$@_test

checker.o: src/checker.c
	$(CC) $(CFLAGS) $^ -c
	@mv $@ lib/

error: src/error.c lib/log.o lib/common.o lib/test.o
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

metric: src/metric.c lib/log.o lib/test.o lib/error.o
	$(CC) $(CFLAGS) -DMETRICTESTING $^ -o test/$@_test

metric.o: src/metric.c
	$(CC) $(CFLAGS) $^ -c
	@mv $@ lib/

array.o: src/array.c
	$(CC) $(CFLAGS) $^ -c
	@mv $@ lib/

array: src/array.c lib/log.o lib/test.o lib/error.o lib/checker.o lib/common.o
	$(CC) $(CFLAGS) -DARRAYTESTING $^ -o test/$@_test

buffer:	src/buffer.c lib/log.o lib/test.o lib/error.o lib/checker.o lib/common.o
	$(CC) $(CFLAGS) -DBUFFERTESTING $^ -o test/$@_test

buffer.o: src/buffer.c
	$(CC) $(CFLAGS)  $^ -c
	@mv $@ lib/

fs:	src/fs.c lib/log.o lib/test.o lib/error.o lib/checker.o lib/common.o
	$(CC) $(CFLAGS) -DFSTESTING $^ -o test/$@_test

fs.o: 	src/fs.c
	$(CC) $(CFLAGS)  $^ -c
	@mv $@ lib/

fs_iter: src/fs_iter.c lib/log.o lib/test.o lib/error.o lib/checker.o lib/common.o lib/fs.o
	$(CC) $(CFLAGS) -DFSITERTESTING $^ -o test/$@_test

fileutils.o: src/fileutils.c
	$(CC) $(CFLAGS)  $^ -c
	@mv $@ lib/

fileutils: src/fileutils.c lib/log.o lib/test.o lib/error.o lib/checker.o lib/common.o lib/fs.o
	 $(CC) $(CFLAGS) -DFILEUTILSTESTING $^ -o test/$@_test


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

