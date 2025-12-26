CC=gcc #clang
CFLAGS= -I../utils -Wall -Wextra -std=gnu11 -g -O0
# -Wno-initializer-overrides -Wno-unused-function -Wno-format-zero-length
LDFLAGS=
LDLIBS=

all:

hello: src/hello.c
	$(CC) $(CFLAGS) $^ -o $@

test_pi: src/test_pi.c
	$(CC) $(CFLAGS) $^ -o out/$@

test1: src/test1.c
	$(CC) $(CFLAGS) $^ -o out/$@

task1.6: src/task1.6.c
	$(CC) $(CFLAGS)  $^ -o out/$@

task1.9: src/task1.9.c
	$(CC) $(CFLAGS)  $^ -o out/$@

1.8.task: src/1.8.task.c
	$(CC) $(CFLAGS)  $^ -o out/$@

1.10.task: src/1.10.task.c
	$(CC) $(CFLAGS)  $^ -o out/$@

1.12.task: src/1.12.task.c
	$(CC) $(CFLAGS)  $^ -o out/$@

UTIL_PATH = ../utils

# libs from utils
lib/log.o:
	cd $(UTIL_PATH) && $(MAKE) log.o
	@cp $(UTIL_PATH)/lib/log.o lib/

lib/test.o:
	cd $(UTIL_PATH) && $(MAKE) test.o
	@cp $(UTIL_PATH)/lib/test.o lib/

lib/error.o:
	cd $(UTIL_PATH) && $(MAKE) error.o
	@cp $(UTIL_PATH)/lib/error.o lib/

lib/common.o:
	cd $(UTIL_PATH) && $(MAKE) common.o
	@cp $(UTIL_PATH)/lib/common.o lib/

.PHONY: clean cleanlog cleanall no_targets__ list
clean:
	-rm *.o
	-rm *_test
	-rm -r *.dSYM
	-rm *.gch
	-rm out/*
	find . -perm -u=x -type f -ls -maxdepth 1 -exec rm {} \;

cleanlog:
	-rm log/*

cleanall: clean cleanlog

no_targets__:
list:
	sh -c "$(MAKE) -p no_targets__ | awk -F':' '/^[a-zA-Z0-9][^\$$#\/\\t=]*:([^=]|$$)/ {split(\$$1,A,/ /);for(i in A)print A[i]}' | grep -v '__\$$' | sort"

