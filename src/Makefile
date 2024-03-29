SOURCES = fusefunc.c dbfuse.c logging.c dbbasic.c dbinit.c dbkey.c dbconsistency.c dbplugin.c dbapriori.c metadata_extract.c plugins_extraction.c import.c apriori.c kwest_main.c

LIBS = -L$(LIB) -lfuse -lsqlite3 -lkw_taglib -lkw_pdfinfo -lkw_extractor -ltag_c -ltag -lm -Wl,-rpath=.

INCLUDE = ../include
LIB = ../lib

EXE = kwest

CC = gcc

CCFLAGS = -g3 -Wall -Wextra -std=gnu99 -pedantic-errors -I$(INCLUDE)

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

kwest_libs: kw_taglib kw_pdfinfo kw_extractor
	export LD_LIBRARY_PATH=$(LIB):$LD_LIBRARY_PATH

kw_taglib: plugin_taglib

plugin_taglib: plugin_taglib.o kwt_upd_meta.o
	gcc -g3 -shared -I$(INCLUDE) -Wl,-soname,libkw_taglib.so -o $(LIB)/libkw_taglib.so -ltag -ltag_c plugin_taglib.o kwt_upd_meta.o

plugin_taglib.o:
	gcc -c -g3 -I$(INCLUDE) -Wall -Wextra -pedantic-errors -std=gnu99 -fPIC -ltag_c -c plugin_taglib.c

kwt_upd_meta.o:
	g++ -c -g3 -I$(INCLUDE) -Wall -Wextra -pedantic-errors -fPIC -ltag kwt_upd_meta.cpp

kw_pdfinfo: plugin_pdfinfo

plugin_pdfinfo: plugin_pdfinfo.o
	gcc -g3 -shared -I$(INCLUDE) -Wl,-soname,libkw_pdfinfo.so -o $(LIB)/libkw_pdfinfo.so -ltag -ltag_c plugin_pdfinfo.o
        
plugin_pdfinfo.o: plugin_pdfinfo.c
	gcc -c -g3 -I$(INCLUDE) -Wall -Wextra -pedantic-errors -std=gnu99 -fPIC -ltag_c -c plugin_pdfinfo.c

kw_extractor: plugin_extractor

plugin_extractor: plugin_libextractor.o
	gcc -g3 -shared -I$(INCLUDE) -Wl,-soname,libkw_extractor.so -o $(LIB)/libkw_extractor.so -ltag -ltag_c plugin_libextractor.o
        
plugin_libextractor.o: plugin_libextractor.c
	gcc -c -g3 -I$(INCLUDE) -Wall -Wextra -pedantic-errors -std=gnu99 -fPIC -ltag_c -c plugin_libextractor.c

c: clean

clean:
	rm -rf *.o
	rm -rf *.db

ca: cleanall

cleanall: clean
	rm -rf $(EXE)

ob: cleanall
	rm -rf ~/.config/$(EXE)/
