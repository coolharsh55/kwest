SOURCES = kwest_main.c fusefunc.c dbfuse.c logging.c dbbasic.c dbinit.c dbkey.c metadata_extract.c plugins_extraction.c import.c

LIBS = -L. -lfuse -lsqlite3 -lkw_taglib -ltag_c

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

c: clean

clean:
	rm -rf *.o
	rm -rf *.db

ca: cleanall
	
cleanall: clean
	rm -rf $(EXE)

ob: cleanall
	rm -rf ~/.config/$(EXE)/
