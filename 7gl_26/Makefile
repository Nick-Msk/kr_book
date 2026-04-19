CC=gcc #clang
CFLAGS= -I../utils -Wall -Wextra -std=gnu11 -g -O0 -Wno-gnu-folding-constant -Wno-initializer-overrides -fmacro-backtrace-limit=0
# -Wno-initializer-overrides -Wno-unused-function -Wno-format-zero-length
LDFLAGS=
LDLIBS=

all:

hash_table.6.5: src/hash_table.6.5.c lib/string_hash.o lib/getword.o lib/log.o lib/common.o lib/fs.o lib/error.o lib/checker.o lib/buffer.o lib/fileutils.o
	$(CC) $(CFLAGS)  $^ -o out/$@

upper_lower.7.1: src/upper_lower.7.1.c
	$(CC) $(CFLAGS)  $^ -o out/$@

formatter.7.2: src/formatter.7.2.c lib/log.o lib/error.o
	$(CC) $(CFLAGS)  $^ -o out/$@

miniprint.7.3: src/miniprint.7.3.c lib/log.o lib/error.o lib/checker.o
	$(CC) $(CFLAGS)  $^ -o out/$@

miniscanf.7.4: src/miniscanf.7.4.c lib/log.o lib/error.o lib/checker.o
	$(CC) $(CFLAGS)  $^ -o out/$@

cat.p7.5: src/cat.p7.5.c lib/log.o lib/error.o lib/checker.o
	$(CC) $(CFLAGS)  $^ -o out/$@

fputs_fgets.p7.6: src/fputs_fgets.p7.6.c lib/log.o lib/error.o lib/checker.o
	$(CC) $(CFLAGS)  $^ -o out/$@

file_differ7.6: src/file_differ7.6.c lib/log.o lib/error.o lib/checker.o lib/fs.o lib/fileutils.o
	$(CC) $(CFLAGS)  $^ -o out/$@

mygrep.7.7: src/mygrep.7.7.c lib/log.o lib/common.o lib/fs.o lib/error.o lib/checker.o lib/fileutils.o
	$(CC) $(CFLAGS)  $^ -o out/$@

printfiles.7.8: src/printfiles.7.8.c lib/log.o lib/common.o lib/fs.o lib/error.o lib/checker.o lib/fileutils.o
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
	cd $(UTIL_PATH) && $(MAKE) fs.o
	@cp $(UTIL_PATH)/lib/fs_array.o lib/

lib/buffer.o:
	cd $(UTIL_PATH) && $(MAKE) buffer.o
	@cp $(UTIL_PATH)/lib/buffer.o lib/

lib/tree_p6.5.o: src/tree_p6.5.c
	$(CC) $(CFLAGS) -I./src  $^ -c
	@mv tree_p6.5.o lib/

lib/tree_6.2.o: src/tree_6.2.c
	$(CC) $(CFLAGS) -I./src  $^ -c
	@mv tree_6.2.o lib/

lib/tree_6.3.o: src/tree_6.3.c
	$(CC) $(CFLAGS) -I./src  $^ -c
	@mv tree_6.3.o lib/

lib/tree_6.4.o: src/tree_6.4.c
	$(CC) $(CFLAGS) -I./src  $^ -c
	@mv tree_6.4.o lib/

# just a regular word + count tree
lib/word_tree.o: src/word_tree.c
	$(CC) $(CFLAGS) -I./src  $^ -c
	@mv word_tree.o lib/

#lib/getword.o: src/getword.c
#	$(CC) $(CFLAGS) -I./src  $^ -c
#	@mv getword.o lib/

lib/string_hash.o: src/string_hash.c
	$(CC) $(CFLAGS) -I./src  $^ -c
	@mv string_hash.o lib/

lib/getword.o:
	cd $(UTIL_PATH) && $(MAKE) getword.o
	@cp $(UTIL_PATH)/lib/getword.o lib/

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

