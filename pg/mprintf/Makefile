#PG_CONFIG = $(HOME)/mypg18.0/bin/pg_config
PG_CONFIG = pg_config
INCLUDEDIR = $(shell $(PG_CONFIG) --includedir-server)
#LIBDIR     = $(shell $(PG_CONFIG) --libdir)
LIBDIR     = $(shell $(PG_CONFIG) --pkglibdir)
SHAREDIR   = $(shell $(PG_CONFIG) --sharedir)/extension
UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
	DLSUFFIX = .dylib
	LDFLAGS = -shared -undefined dynamic_lookup -install_name @rpath/pg_printf.dylib
else
	DLSUFFIX = .so
	LDFLAGS = -shared
endif

TARGET = pg_printf$(DLSUFFIX)

all: $(TARGET) pg_printf--1.0.sql

pg_printf.o: pg_printf.c
	gcc -I$(INCLUDEDIR) -fPIC -c $<

$(TARGET): pg_printf.o
	gcc $(LDFLAGS) -o $@ $^

pg_printf--1.0.sql: printf.sql
	cat $^ > $@

# install зависит от all, поэтому сначала всё соберётся
install: all
	cp $(TARGET) $(LIBDIR)/
	cp pg_printf.control $(SHAREDIR)/
	cp pg_printf--1.0.sql $(SHAREDIR)/

clean:
	rm -f pg_printf.o $(TARGET) pg_printf--1.0.sql

.PHONY: all install clean

