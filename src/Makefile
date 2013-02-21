kwest : kwest_main.o fusefunc.o dbfuse.o logging.o dbbasic.o dbinit.o dbkey.o extract_metadata.o extract_audio_taglib.o import.o
	gcc -o kwest kwest_main.o fusefunc.o dbfuse.o logging.o dbbasic.o dbinit.o dbkey.o extract_metadata.o extract_audio_taglib.o import.o -lfuse -lsqlite3 -ltag_c
 
kwest_main.o : kwest_main.c
	gcc -c -Wall -std=gnu99 kwest_main.c -D_FILE_OFFSET_BITS=64 -lfuse
fusefunc.o: fusefunc.c
	gcc -c -Wall -std=gnu99 fusefunc.c -D_FILE_OFFSET_BITS=64 -lfuse
import.o : import.c
	gcc -c -Wall -std=gnu99 import.c -lsqlite3
extract_audio_taglib.o : extract_audio_taglib.c
	gcc -c -Wall -std=gnu99 extract_audio_taglib.c -ltag_c
dbbasic.o: dbbasic.c
	gcc -c -Wall -std=gnu99 dbbasic.c -lsqlite3
dbinit.o: dbinit.c
	gcc -c -Wall -std=gnu99 dbinit.c -lsqlite3
dbkey.o: dbkey.c
	gcc -c -Wall -std=gnu99 dbkey.c -lsqlite3
extract_metadata.o: extract_metadata.c
	gcc -c -Wall -std=gnu99 extract_metadata.c -lsqlite3
logging.o: logging.c
	gcc -c -Wall -std=gnu99 logging.c
dbfuse.o: dbfuse.c
	gcc -c -Wall -std=gnu99 dbfuse.c 

test: test.o plugins_extraction.o metadata_extract.o plugin_taglib.o
	gcc -o test -std=gnu99 test.o plugins_extraction.o metadata_extract.o plugin_taglib.o

test.o: test.c
	gcc -c -Wall -Wextra -pedantic -std=gnu99 test.c

plugins_extraction.o : plugins_extraction.c
	gcc -c -Wall -Wextra -pedantic -std=gnu99 plugins_extraction.c

metadata_extract.o : metadata_extract.c
	gcc -c -Wall -Wextra -pedantic -std=gnu99 metadata_extract.c

plugin_taglic.o : plugin_taglic.c
	gcc -c -Wall -Wextra -pedantic -std=gnu99 plugin_taglib.c

cleantest:
	rm -rf *.o
	rm -rf test

clean:
	rm -rf *.o
	rm -rf *.db
	rm -rf kwest
	
cleanall:
	rm -rf *.o
	rm -rf *.db
	rm -rf kwest
	rm -rf ~/.config/kwest/

