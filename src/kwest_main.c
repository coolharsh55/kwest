/**
 * @file kwest_main.c
 * @brief kwest main function
 * @author Harshvardhan Pandit
 * @date March 2013
 */

 /** @mainpage
 * @tableofcontents
 * @section license LICENSE
 * Copyright 2013 Harshvardhan Pandit
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * 	http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @section running RUNNING
 * all the files are currently in "src" with the makefile
makefile commands are:
@code
make
make kwest_libs - compile and create kwest shared libraries
make clean - clean compiled object files
make cleanall - clean compiled files, and executables
make ob - clean compiled objects, files, executables and kwest config directory
@endcode
creates executable "kwest"
@code
$mkdir mnt
$./kwest mnt
./kwest is the executable
mnt is the mountpoint
similar to regular mounting from devices
$cd mnt
$mnt: ls
$mnt browse as filesystem
when need to unmount, return to "parent" of "mnt"
$fusermount -u mnt
@endcode

@subsection dependson Dependencies:
@code
gcc
fuse version 2.8+
	$sudo apt-get install fuse libfuse-dev
sqlite3 3.7.0+
	$sudo apt-get install sqlite3 libsqlite3-dev
taglib 1.7+
	$sudo apt-get install libtag-dev libtagc0 libtagc0-dev libtag-extras1 libtag-extras1-dev
@endcode


 * @section DEBUGGING
 * @subsection valgrind Valgrind
 * memory check kwest through:
 * @code
 * valgrind --tool=memcheck --trace-children=no --leak-check=full --track-origins=yes -v ./kwest mnt -d -f -s
 * $ cd /usr/bin
 * $ if [[ -e fusermount ]]; then mv fusermount fusermount.real; fi
 * $ touch fusermount
 * $ chown root.fuse fusermount
 * $ chmod ug+x fusermount
 * $ echo '#!/bin/sh' >> fusermount
 * $ echo 'exec /usr/bin/fusermount.real $@' >> fusermount
 * or depending on the path of fusermount
 * $ echo 'exec /bin/fusermount $@' >> fusermount
 * @endcode
 *
 * @subsection gdb GDB
 * check stacks and execution control through
 * @code
 * $ gdm kwest
 * gdb execution comments
 * _ run -d -s mnt
 * @endcode
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>

#include "fusefunc.h"
#include "dbinit.h"
#include "apriori.h"
#include "dbconsistency.h"
#include "import.h"
#include "logging.h"
#include "flags.h"
#include "magicstrings.h"

#include "plugins_extraction.h"
#include "plugin_taglib.h"
#include "plugin_pdfinfo.h"
#include "plugin_libextractor.h"


/**
 * @brief kwest main function
 * @author Harshvardhan Pandit
 * @param argc argument count
 * @param argv argument variables
 * @return int fuse return value
 */
int main(int argc, char *argv[])
{
	/** get user uid */
	struct passwd *pw = getpwuid(getuid());
	int ret;
	/** get user home directory */
	const char *homedir = pw->pw_dir;
	FILE *stderror = NULL;
	char logdir[QUERY_SIZE];
	char musicdir[QUERY_SIZE];
	/** set current scan directory to Music */
	strcpy(musicdir, homedir);
	strcat(musicdir,"/Music");
	strcpy(logdir, homedir);
	strcat(logdir, CONFIG_LOCATION);
	strcat(logdir, LOGFILE_STORAGE);

	printf("KWEST - A Semantically Tagged Virtual File System\n");
	printf("Initiating logging file(s).........\n");
	/** initialize logging mechanism */
	if(log_init() == KW_SUCCESS) {
		stderror = freopen(logdir, "w", stderr);
		if(stderror == NULL) { /* redirect stderr to logfile */
			printf("could not redirect stderror\n");
		}
		log_msg("KWEST - A Semantically Tagged Virtual File System");
		log_msg("program initiated");
		printf("SUCCESS!\n");

	} else {
		printf("FAILED\n");
		printf("Exiting program...\n");
		return -1;
	}
	/** initialize database */
	begin_transaction();
	create_db();
	commit_transaction();
	/** load plugins */
	begin_transaction();
	ret = plugins_add_plugin(load_taglib_plugin());
	printf("plugin load status = %d\n", ret);
	ret = plugins_add_plugin(load_pdfinfo_plugin());
	printf("plugin load status = %d\n", ret);
	ret = plugins_add_plugin(load_libextractor_plugin());
	printf("plugin load status = %d\n", ret);
	commit_transaction();

	begin_transaction();
	/** Validate Database entries */
	check_db_consistency();
	/** import files into kwest */
	printf("Importing file from %s\n",musicdir);
	if(import(musicdir) == KW_SUCCESS) {
		log_msg("Importing files = SUCCESS");
		printf("Import completed SUCCESSFULLY\n");
	} else {
		log_msg("Importing files = FAILED");
		printf("FAILED");
		printf("Exiting program...\n");
		return -1;
	}
	apriori();
	commit_transaction();
	/** restore stderr to default */
	if(stderror != NULL) { /* restore stderr to stdout */
		stderr = stderror;
	}
	/** pass control to fuse daemon */
	return call_fuse_daemon(argc,argv);
	/** file system is now LIVE */
}
