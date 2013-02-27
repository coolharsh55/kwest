SOURCES = kwest_main.c fusefunc.c dbfuse.c logging.c dbbasic.c dbinit.c dbkey.c metadata_extract.c plugins_extraction.c import.c

LIBS = -L$(LIB) -lfuse -lsqlite3 -lkw_taglib -ltag_c -ltag -Wl,-rpath=.

INCLUDE = ../include
LIB = ../lib

EXE = kwest

CC = gcc

CCFLAGS = -g -Wall -Wextra -std=gnu99 -pedantic-errors -I$(INCLUDE)

OFLAGS = -c

ARCH = $(shell getconf LONG_BIT)

X = -D_FILE_OFFSET_BITS=$(ARCH)

OBJECTS = $(SOURCES:.c=.o)

$(EXE) : $(OBJECTS)
	$(CC) -o $(EXE) $(OBJECTS) $(LIBS)

%.o: %.c
	$(CC) $(OFLAGS) $(CCFLAGS) $< 
 
fusefunc.o: fusefunc.c
	$(CC) $(OFLAGS) $(CCFLAGS) $< $X

kwest_libs: kw_taglib
	export LD_LIBRARY_PATH=$(LIB):$LD_LIBRARY_PATH

kw_taglib: plugin_taglib

plugin_taglib: plugin_taglib.o kwt_upd_meta.o
	gcc -shared -I./include -Wl,-soname,libkw_taglib.so -o $(LIB)/libkw_taglib.so -ltag -ltag_c plugin_taglib.o kwt_upd_meta.o

plugin_taglib.o:
	gcc -c -I./include -Wall -Wextra -pedantic -std=gnu99 -fPIC -ltag_c -c plugin_taglib.c

kwt_upd_meta.o:
	g++ -c -Wall -Wextra -pedantic -fPIC -ltag kwt_upd_meta.cpp

c: clean

clean:
	rm -rf *.o
	rm -rf *.db

ca: cleanall
	
cleanall: clean
	rm -rf $(EXE)

ob: cleanall
	rm -rf ~/.config/$(EXE)/
