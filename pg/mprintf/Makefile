PG_CONFIG = $(HOME)/mypg18.0/bin/pg_config
INCLUDEDIR = $(shell $(PG_CONFIG) --includedir-server)
#LIBDIR     = $(shell $(PG_CONFIG) --libdir)
LIBDIR     = $(shell $(PG_CONFIG) --pkglibdir)
SHAREDIR   = $(shell $(PG_CONFIG) --sharedir)/extension

all: pg_printf.so pg_printf--1.0.sql

pg_printf.o: pg_printf.c
	gcc -I$(INCLUDEDIR) -fPIC -c $<

pg_printf.so: pg_printf.o
	gcc -shared -o $@ $^

pg_printf--1.0.sql: printf.sql
	cat $^ > $@

# install зависит от all, поэтому сначала всё соберётся
install: all
	cp pg_printf.so $(LIBDIR)/
	cp pg_printf.control $(SHAREDIR)/
	cp pg_printf--1.0.sql $(SHAREDIR)/

clean:
	rm -f pg_printf.o pg_printf.so pg_printf--1.0.sql

.PHONY: all install clean

