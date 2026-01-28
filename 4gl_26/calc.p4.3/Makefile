CC=gcc
CFLAGS= -I../../utils -Isrc/ -Wall -Wextra -std=gnu11 -g -O0 -Wno-gnu-folding-constant
# -Wno-initializer-overrides -Wno-unused-function -Wno-format-zero-length
LDFLAGS=
LDLIBS=

calc: src/calc.c lib/log.o lib/common.o lib/checker.o lib/stack.o lib/getop.o lib/var.o
	$(CC) $(CFLAGS)  $^ -o out/$@

lib/stack.o: src/stack.c
	$(CC) $(CFLAGS)  $^ -c
	@mv stack.o lib/

stack: src/stack.c lib/log.o lib/test.o lib/error.o lib/array.o lib/checker.o
	$(CC) $(CFLAGS) -DSTACKTESTING $^ -o test/$@_test

getop: src/getop.c lib/log.o lib/test.o lib/error.o lib/checker.o lib/buffer.o
	$(CC) $(CFLAGS) -DGETOPTESTING $^ -o test/$@_test

lib/getop.o: src/getop.c
	$(CC) $(CFLAGS)  $^ -c
	@mv getop.o lib/

var: src/var.c lib/log.o lib/test.o lib/error.o lib/checker.o lib/common.o
	$(CC) $(CFLAGS) -DVARTESTING $^ -o test/$@_test

lib/var.o: src/var.c
	$(CC) $(CFLAGS)  $^ -c
	@mv var.o lib/

buffer: src/buffer.c lib/log.o lib/test.o lib/error.o lib/checker.o lib/common.o
	$(CC) $(CFLAGS) -DBUFFERTESTING $^ -o test/$@_test

lib/buffer.o: src/buffer.c
	$(CC) $(CFLAGS)  $^ -c
	@mv buffer.o lib/

UTIL_PATH = ../../utils

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

lib/checker.o:
	cd $(UTIL_PATH) && $(MAKE) checker.o
	@cp $(UTIL_PATH)/lib/checker.o lib/

lib/metric.o:
	cd $(UTIL_PATH) && $(MAKE) metric.o
	@cp $(UTIL_PATH)/lib/metric.o lib/

lib/array.o:
	cd $(UTIL_PATH) && $(MAKE) array.o
	@cp $(UTIL_PATH)/lib/array.o lib/

.PHONY: clean cleanlog cleanall no_targets__ list
clean:
	-rm src/*.c~
	-rm src/.*.un~
	-rm -rf out/*
	-rm lib/*
	@find . -perm -u=x -type f -ls -maxdepth 1 -exec rm {} \;

cleanlog:
	-rm log/*

cleanall: clean cleanlog

no_targets__:
list:
	sh -c "$(MAKE) -p no_targets__ | awk -F':' '/^[a-zA-Z0-9][^\$$#\/\\t=]*:([^=]|$$)/ {split(\$$1,A,/ /);for(i in A)print A[i]}' | grep -v '__\$$' | sort"

