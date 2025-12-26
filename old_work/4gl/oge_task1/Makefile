CC=gcc #clang
CFLAGS= -I/Users/skelet/code/c/kr_book/utils -Wall -Wextra -std=gnu11 -g -O0 -Wno-initializer-overrides
# -Wno-initializer-overrides -Wno-unused-function -Wno-format-zero-length
LDFLAGS=
LDLIBS=

all: task5

task5: src/task5.c lib/log.o
	$(CC) $(CFLAGS)  $^ -o debug/$@ 

lib/log.o:
	cd /Users/skelet/code/c/kr_book/utils/ && $(MAKE) log
	#$(CC) $(CFLAGS)  $^ -c
	@cp /Users/skelet/code/c/kr_book/utils/log.o lib/

run:
	debug/task5

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


