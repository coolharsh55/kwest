/**
 * @file dbconsistency.c
 * @brief Maintain database consistency
 * @author Sahil Gupta
 * @date Feb 2012
 */

/* LICENSE
 * Copyright 2013 Sahil Gupta
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
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <dirent.h>

#include "dbconsistency.h"
#include "dbbasic.h"
#include "dbinit.h"
#include "dbkey.h"
#include "logging.h"
#include "flags.h"
#include "magicstrings.h"


/**
 * @brief Remove FileSystem tag if empty
 * @param abspath Absolute path of file
 * @return void
 * @author SG
 */
static void check_tag_if_empty(const char *abspath)
{
	DIR *directory;

	/* Extract tagname from absolute path of file */
	char *tagpath = strdup(abspath);
	char *tagname = strrchr(tagpath,'/');
	*tagname = '\0';
	tagname = strrchr(tagpath,'/') + 1;

	/* Remove Tag if Directory deleted from File System */
	if((directory = opendir(tagpath)) == NULL) {
		if(remove_tag(tagname) == KW_SUCCESS) {
			log_msg("Removing tag : %s",tagname);
			printf("Removing tag : %s\n",tagname);
		}
		free(tagpath);
		return;
	} else {
		closedir(directory);
	}

	/* Remove tag if no files in tag */
	/*
	sqlite3_stmt* stmt;
	char query[QUERY_SIZE];
	int status;
	int tno;

	tno = get_tag_id(tagname);

	sprintf(query,"select count(*) from FileAssociation where tno = %d;",
		      tno);
	status = sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	status = sqlite3_step(stmt);
	if(status == SQLITE_ROW) {
		if(atoi((const char*)sqlite3_column_text(stmt,0)) == 0) {
			log_msg("Removing tag : %s",tagname);
			printf("Removing tag : %s\n",tagname);
			remove_tag(tagname);
		}
	}
	sqlite3_finalize(stmt);
	*/

	free(tagpath);
}

/**
 * @brief Check if files in database exist on file system
 * @param void
 * @return KW_SUCCESS: SUCCESS, KW_ERROR: ERROR
 * @author SG
 */
int check_db_consistency(void)
{
	sqlite3_stmt* stmt;
	char query[QUERY_SIZE];
	int status;
	const char *tmp; /* Holds abspath */

	log_msg("Checking database consistency\n");
	strcpy(query,"select abspath from FileDetails;");
	status = sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	if(status != SQLITE_OK) { /* Error Preparing query */
		return KW_ERROR;
	}

	do {
		status = sqlite3_step(stmt);
		if(status == SQLITE_ROW) {
			tmp = (const char*)sqlite3_column_text(stmt,0);
			FILE *f=fopen(tmp,"rb");
			if(f == NULL) { /* Remove from Database */
				check_tag_if_empty(tmp);
				remove_file(tmp);
				log_msg("Removing file : %s",tmp);
				printf("Removing file : %s\n",tmp);
			} else {
				fclose(f);
			}
		}
	}while(status == SQLITE_ROW);
	sqlite3_finalize(stmt);

	return KW_SUCCESS;
}
