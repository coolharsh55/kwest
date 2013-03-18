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
	sqlite3_stmt *stmt_tno,*stmt_fcnt;
	char query[QUERY_SIZE];
	int status_fcnt;
	const char *tagname;
	int tno;
	int fno;

	fno = get_file_id(strrchr(abspath,'/') + 1);

	sprintf(query,"select tno from FileAssociation where fno = %d;",fno);
	sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt_tno,0);

	do {
		/* Get individual rows */
		if(SQLITE_ROW != sqlite3_step(stmt_tno)) {
			break;
		}

		tno = atoi((char*)sqlite3_column_text(stmt_tno,0));
		sprintf(query,"select count(*) from FileAssociation where tno"
		              " = %d;", tno);
		sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt_fcnt,0);

		status_fcnt = sqlite3_step(stmt_fcnt);
		if(status_fcnt == SQLITE_ROW) {
			if(atoi((const char*)sqlite3_column_text(stmt_fcnt,0))
			   == 1) {
				tagname = get_tag_name(tno);
				log_msg("Removing tag : %s",tagname);
				printf("Removing tag : %s\n",tagname);
				remove_tag((char *)tagname);
				free((char *)tagname);
			}
		}
		sqlite3_finalize(stmt_fcnt);

	}while(1);
	sqlite3_finalize(stmt_tno);
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
