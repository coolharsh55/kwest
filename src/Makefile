SOURCES = kwest_main.c fusefunc.c dbfuse.c logging.c dbbasic.c dbinit.c dbkey.c metadata_extract.c plugins_extraction.c import.c

LIBS = -L. -lfuse -lsqlite3 -lkw_taglib -ltag_c -ltag -Wl,-rpath=.

EXE = kwest

CC = gcc

CCFLAGS = -g -Wall -Wextra -std=gnu99 -pedantic-errors

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

test:
	export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH

kw_taglib: plugin_taglib

plugin_taglib: plugin_taglib.o kwt_upd_meta.o
	gcc -shared -Wl,-soname,libkw_taglib.so -o libkw_taglib.so -ltag -ltag_c plugin_taglib.o kwt_upd_meta.o

plugin_taglib.o:
	gcc -c -Wall -Wextra -pedantic -std=gnu99 -fPIC -ltag_c -c plugin_taglib.c

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
