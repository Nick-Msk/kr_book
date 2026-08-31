CC=gcc #clang
CFLAGS= -I/Users/skelet/code/c/kr_book/utils -Wall -Wextra -std=gnu11 -g -O0 -Wno-initializer-overrides
# -Wno-initializer-overrides -Wno-unused-function -Wno-format-zero-length
LDFLAGS=
LDLIBS=

all: calc_rp

calc_rp: lib/calc_rp.4.3.o lib/log.o lib/var.o lib/buffer.o lib/stack.o lib/getop.o lib/common.o
	$(CC) $(CFLAGS)  $^ -o debug/$@

lib/calc_rp.4.3.o: src/calc_rp.4.3.c
	$(CC) $(CFLAGS)  $^ -c
	@mv calc_rp.4.3.o lib/

lib/var.o: src/var.c src/var.h
	$(CC) $(CFLAGS)  $^ -c
	@mv var.o lib/

lib/buffer.o: src/buffer.c src/buffer.h
	$(CC) $(CFLAGS)  $^ -c
	@mv buffer.o lib/

lib/stack.o: src/stack.c src/stack.h
	$(CC) $(CFLAGS)  $^ -c
	@mv stack.o lib/

lib/getop.o: src/getop.c src/getop.h lib/buffer.o
	$(CC) $(CFLAGS)  $^ -c
	@mv getop.o lib/

# libs from utils
lib/log.o:
	cd /Users/skelet/code/c/kr_book/utils/ && $(MAKE) log
	#$(CC) $(CFLAGS)  $^ -c
	@cp /Users/skelet/code/c/kr_book/utils/log.o lib/

lib/test.o:
	cd /Users/skelet/code/c/kr_book/utils/ && $(MAKE) test
	#$(CC) $(CFLAGS)  $^ -c
	@cp /Users/skelet/code/c/kr_book/utils/test.o lib/

lib/error.o:
	cd /Users/skelet/code/c/kr_book/utils/ && $(MAKE) error
	#$(CC) $(CFLAGS)  $^ -c
	@cp /Users/skelet/code/c/kr_book/utils/error.o lib/

lib/common.o:
	cd /Users/skelet/code/c/kr_book/utils/ && $(MAKE) error
	 #$(CC) $(CFLAGS)  $^ -c
	@cp /Users/skelet/code/c/kr_book/utils/common.o lib/


#testing for var.c, I'm unable to add var.h as dependency here! TODO: check that
var: src/var.c lib/test.o lib/log.o lib/error.o
	$(CC) $(CFLAGS) -DVARTESTING $^ -o test/$@_test

#testing buffer.c
buffer: src/buffer.c lib/test.o lib/log.o lib/error.o
	$(CC) $(CFLAGS) -DBUFFERTESTING $^ -o test/$@_test

#testing stack.c
stack: src/stack.c lib/test.o lib/log.o lib/error.o
	$(CC) $(CFLAGS) -DSTACKTESTING $^ -o test/$@_test

#testing getop.c
getop: src/getop.c lib/test.o lib/log.o lib/error.o lib/buffer.o
	$(CC) $(CFLAGS) -DGETOPTESTING $^ -o test/$@_test

# methods
run:
	@debug/rpolskn

vartest:
	@test/var_test

buffertest:
	@test/buffer_test

stacktest:
	@test/stack_test

getoptest:
	@test/getop_test

#empty for now
test:

.PHONY: clean cleanlog cleanall no_targets__ list
clean:
	@-rm lib/*.o
	@-rm test/*_test
	@find . -name "*.dSYM" -type d -ls -maxdepth 2 -exec rm -r {} \;
	@find . -perm -u=x -type f -ls -maxdepth 2 -exec rm {} \;
	@find . -name "*.gch" -type f -maxdepth 2 -exec rm {} \;
	@find . -name "*.h~" -type f -maxdepth 2 -exec rm {} \;
	@find . -name "*.c~" -type f -maxdepth 2 -exec rm {} \;
	@find . -name "Makefile~" -type f -maxdepth 1 -exec rm {} \;

cleanlog:
	-rm *.log

cleanall: clean cleanlog

no_targets__:
list:
	sh -c "$(MAKE) -p no_targets__ | awk -F':' '/^[a-zA-Z0-9][^\$$#\/\\t=]*:([^=]|$$)/ {split(\$$1,A,/ /);for(i in A)print A[i]}' | grep -v '__\$$' | sort"

