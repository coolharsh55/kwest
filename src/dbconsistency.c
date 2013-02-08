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

#include "dbconsistency.h"
#include "dbbasic.h"
#include "dbinit.h"
#include "dbkey.h"
#include "logging.h"
#include "flags.h"
#include "magicstrings.h"


/**
 * @brief Check if files in database exist on file system
 * @param void
 * @return KW_SUCCESS: SUCCESS, KW_ERROR: ERROR
 * @author SG
 * @todo Remove Tag
 */
int check_db_consistency(void)
{
	sqlite3_stmt* stmt; 
	char query[QUERY_SIZE];
	int status;
	const char *tmp; /* Holds abspath */
	
	log_msg("Checking database consistency");
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
				log_msg("Removing file : %s",tmp);
				printf("Removing file : %s\n",tmp);
				remove_file(get_file_id_by_abspath(tmp));
			} else {
				fclose(f);
			}
		}
	}while(status == SQLITE_ROW);
	sqlite3_finalize(stmt);
	
	return KW_SUCCESS;
}
