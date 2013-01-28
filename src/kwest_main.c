/* kwest_main.c
 * main function
 */

/* LICENSE
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
 */



#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>
#include "import.h"
#include "fusefunc.h"
/* #include "db_consistency.h" */
#include "logging.h"
#include "flags.h"
#include "dbinit.h"
#include "import.h"
#include "extract_metadata.h"
#include "magicstrings.h"


/* kwest main function
 */
int main(int argc, char *argv[])
{
	struct passwd *pw = getpwuid(getuid());
	const char *homedir = pw->pw_dir;
	FILE *stderror = NULL;
	char logdir[QUERY_SIZE];
	char musicdir[QUERY_SIZE];
	
	strcpy(musicdir, homedir);
	strcat(musicdir,"/Music");
	strcpy(logdir, homedir);
	strcat(logdir, CONFIG_LOCATION);
	strcat(logdir, LOGFILE_STORAGE);
	
	stderror = freopen(logdir, "w", stderr); 
	if(stderror == NULL) { /* redirect stderr to logfile */
		printf("could not redirect stderror\n");
	}
	
	printf("KWEST - A Semantically Tagged Virtual File System\n");
	printf("Initiating logging file(s).........\n");
	if(log_init() == KW_SUCCESS) {
		log_msg("KWEST - A Semantically Tagged Virtual File System");
		log_msg("program initiated");
		printf("SUCCESS!\n");
		
	} else {
		printf("FAILED\n");
		printf("Exiting program...\n");
		return -1;
	}
	begin_transaction();
	create_db();
	
	audio_metadata();
	
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
	
	commit_transaction();
	if(stderror != NULL) { /* restore stderr to stdout */
		stderr = stderror;
	}
	
	return call_fuse_daemon(argc,argv);
}
