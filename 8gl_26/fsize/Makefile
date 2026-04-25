CC=gcc #clang
CFLAGS= -I../../utils -Wall -Wextra -std=gnu11 -g -O0 -Wno-gnu-folding-constant -Wno-initializer-overrides -fmacro-backtrace-limit=0
# -Wno-initializer-overrides -Wno-unused-function -Wno-format-zero-length
LDFLAGS=
LDLIBS=

all:

fsize.p8.6: src/fsize.p8.6.c lib/log.o lib/common.o lib/error.o lib/checker.o lib/fsize.o lib/fs.o lib/fileutils.o
	$(CC) $(CFLAGS)  $^ -o out/$@

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

lib/fileutils.o:
	cd $(UTIL_PATH) && $(MAKE) fileutils.o
	@cp $(UTIL_PATH)/lib/fileutils.o lib/

lib/fs.o:
	cd $(UTIL_PATH) && $(MAKE) fs.o
	@cp $(UTIL_PATH)/lib/fs.o lib/

lib/fs_array.o:
	cd $(UTIL_PATH) && $(MAKE) fs_array.o
	@cp $(UTIL_PATH)/lib/fs_array.o lib/

lib/fs_iter.o:
	cd $(UTIL_PATH) && $(MAKE) fs_iter.o
	@cp $(UTIL_PATH)/lib/fs_iter.o lib/

lib/buffer.o:
	cd $(UTIL_PATH) && $(MAKE) buffer.o
	@cp $(UTIL_PATH)/lib/buffer.o lib/

lib/fsize.o: src/fsize.c
	$(CC) $(CFLAGS) -I./src  $^ -c
	@mv fsize.o lib/

lib/getword.o:
	cd $(UTIL_PATH) && $(MAKE) getword.o
	@cp $(UTIL_PATH)/lib/getword.o lib/

lib/command_executor.o:
	cd $(UTIL_PATH) && $(MAKE) command_executor.o
	@cp $(UTIL_PATH)/lib/command_executor.o lib/

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

