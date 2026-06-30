CC = gcc
CFLAGS = -I../utils -Wall -Wextra -std=gnu11 -g -O0 \
         -Wno-gnu-folding-constant -Wno-initializer-overrides \
         -fmacro-backtrace-limit=0 -fsanitize=address
LDFLAGS =
LDLIBS =

UTIL_PATH = ../utils
UTIL_SRC  = $(UTIL_PATH)/src

# ------------- Phony-алиасы для исполняемых файлов -------------
.PHONY: file_ops.p8.2 cp.p8.3 cat.8.1 files_checker mcp kaprekar triplets
.PHONY: all clean cleanlog cleanall no_targets__ list

file_ops.p8.2: out/file_ops.p8.2
cp.p8.3:       out/cp.p8.3
cat.8.1:       out/cat.8.1
files_checker: out/files_checker
mcp:           out/mcp
kaprekar:      out/kaprekar
triplets:      out/triplets

all: file_ops.p8.2 cp.p8.3 cat.8.1 files_checker mcp kaprekar triplets

# ------------- Исполняемые файлы (сборка в out/) -------------
out/file_ops.p8.2: src/file_ops.p8.2.c lib/log.o lib/common.o lib/error.o lib/checker.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

out/cp.p8.3: src/cp.p8.3.c lib/log.o lib/common.o lib/error.o lib/checker.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

out/cat.8.1: src/cat.8.1.c lib/log.o lib/common.o lib/error.o lib/checker.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

out/files_checker: src/files_checker.c lib/log.o lib/common.o lib/error.o lib/checker.o \
                   lib/files.p8.5.o lib/getword.o lib/buffer.o lib/fs_iter.o \
                   lib/fs.o lib/fileutils.o lib/proc_actions.o lib/command_executor.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

out/mcp: src/mcp.c lib/log.o lib/common.o lib/error.o lib/checker.o lib/files.p8.5.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

out/kaprekar: src/kaprekar.c lib/log.o lib/common.o lib/error.o lib/checker.o \
              lib/fs.o lib/fileutils.o lib/getword.o lib/buffer.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

out/triplets: src/triplets.c lib/log.o lib/common.o lib/error.o lib/checker.o
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

# ------------- Локальные объектные файлы (сразу в lib/) -------------
lib/files.p8.5.o: src/files.p8.5.c
	$(CC) $(CFLAGS) -c $< -o $@

lib/proc_actions.o: src/proc_actions.c
	$(CC) $(CFLAGS) -c $< -o $@

lib/tree_p6.5.o: src/tree_p6.5.c
	$(CC) $(CFLAGS) -I./src -c $< -o $@

lib/tree_6.2.o: src/tree_6.2.c
	$(CC) $(CFLAGS) -I./src -c $< -o $@

lib/tree_6.3.o: src/tree_6.3.c
	$(CC) $(CFLAGS) -I./src -c $< -o $@

lib/tree_6.4.o: src/tree_6.4.c
	$(CC) $(CFLAGS) -I./src -c $< -o $@

lib/word_tree.o: src/word_tree.c
	$(CC) $(CFLAGS) -I./src -c $< -o $@

lib/string_hash.o: src/string_hash.c
	$(CC) $(CFLAGS) -I./src -c $< -o $@

# ------------- Объектные файлы из ../utils (с отслеживанием исходников) -------------
lib/log.o: $(UTIL_SRC)/log.c
	cd $(UTIL_PATH) && $(MAKE) lib/log.o
	cp $(UTIL_PATH)/lib/log.o $@

lib/test.o: $(UTIL_SRC)/test.c
	cd $(UTIL_PATH) && $(MAKE) lib/test.o
	cp $(UTIL_PATH)/lib/test.o $@

lib/error.o: $(UTIL_SRC)/error.c
	cd $(UTIL_PATH) && $(MAKE) lib/error.o
	cp $(UTIL_PATH)/lib/error.o $@

lib/common.o: $(UTIL_SRC)/common.c
	cd $(UTIL_PATH) && $(MAKE) lib/common.o
	cp $(UTIL_PATH)/lib/common.o $@

lib/checker.o: $(UTIL_SRC)/checker.c
	cd $(UTIL_PATH) && $(MAKE) lib/checker.o
	cp $(UTIL_PATH)/lib/checker.o $@

lib/metric.o: $(UTIL_SRC)/metric.c
	cd $(UTIL_PATH) && $(MAKE) lib/metric.o
	cp $(UTIL_PATH)/lib/metric.o $@

lib/array.o: $(UTIL_SRC)/array.c
	cd $(UTIL_PATH) && $(MAKE) lib/array.o
	cp $(UTIL_PATH)/lib/array.o $@

lib/fileutils.o: $(UTIL_SRC)/fileutils.c
	cd $(UTIL_PATH) && $(MAKE) lib/fileutils.o
	cp $(UTIL_PATH)/lib/fileutils.o $@

lib/fs.o: $(UTIL_SRC)/fs.c
	cd $(UTIL_PATH) && $(MAKE) lib/fs.o
	cp $(UTIL_PATH)/lib/fs.o $@

lib/fs_array.o: $(UTIL_SRC)/fs_array.c
	cd $(UTIL_PATH) && $(MAKE) lib/fs_array.o
	cp $(UTIL_PATH)/lib/fs_array.o $@

lib/fs_iter.o: $(UTIL_SRC)/fs_iter.c
	cd $(UTIL_PATH) && $(MAKE) lib/fs_iter.o
	cp $(UTIL_PATH)/lib/fs_iter.o $@

lib/buffer.o: $(UTIL_SRC)/buffer.c
	cd $(UTIL_PATH) && $(MAKE) lib/buffer.o
	cp $(UTIL_PATH)/lib/buffer.o $@

lib/getword.o: $(UTIL_SRC)/getword.c
	cd $(UTIL_PATH) && $(MAKE) lib/getword.o
	cp $(UTIL_PATH)/lib/getword.o $@

lib/command_executor.o: $(UTIL_SRC)/command_executor.c
	cd $(UTIL_PATH) && $(MAKE) lib/command_executor.o
	cp $(UTIL_PATH)/lib/command_executor.o $@

clean:
	-rm -f src/*.c~ src/.*.un~
	-rm -rf out/*
	-rm -rf lib/*
	@find . -perm -u=x -type f -maxdepth 1 -exec rm {} \;

cleanlog:
	-rm -rf log/*

cleanall: clean cleanlog

no_targets__:
list:
	sh -c "$(MAKE) -p no_targets__ | awk -F':' '/^[a-zA-Z0-9][^\$$#\/\\t=]*:([^=]|$$)/ {split(\$$1,A,/ /);for(i in A)print A[i]}' | grep -v '__\$$' | sort"
