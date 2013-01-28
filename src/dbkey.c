/* dbkey.c
 * handle requests for files and tags
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
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>    /* SQLITE Functions */

#include "dbinit.h"

#include "dbkey.h"
#include "flags.h"


/* get_field_count
 * Return count of entries in database
 * 
 * @param const char *querystring 
 * @return int count
 * @author @SG
 */
static int get_field_count(const char *querystring)
{
	sqlite3_stmt *stmt = NULL;
	char query[QUERY_SIZE];
	int status;
	
	/* Query to get field count */
	strcpy(query, querystring);
	sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);
	
	status = sqlite3_step(stmt);
	if(status == SQLITE_ROW) { /* Return field count */
		status = atoi((const char*)sqlite3_column_text(stmt,0));
		sqlite3_finalize(stmt);
		return status;
	}
	
	sqlite3_finalize(stmt);
	return KW_FAIL;
}

/* set_file_id: Generate id for new file to be added in kwest
 * param: char *abspath - Absolute Path of File
 * return: int fno
 * author: @SG 
 */
int set_file_id(const char *abspath)
{
	int tmp;
	
	/* Check if file exists */
	tmp = get_file_id((strrchr(abspath,'/')+1)); /* Get File ID */
	if(tmp != KW_FAIL){ 
		return KW_FAIL; /* Return if file already exists */
	}
	
	/* Query to get maximum no of files existing in database */
	tmp = get_field_count("select count(fno) from FileDetails;");
	
	if(tmp == KW_FAIL) {
		return KW_FAIL;
	} else if(tmp == NO_DB_ENTRY) { /* No Entry */
		return FILE_START;
	} else { /* Increment File ID */
		return (tmp + 1);
	}
}


/* set_tag_id: Generate id for a new tag to be created in kwest
 * param: char *tagname
 * return: int tno 
 * author: @SG 
 */
int set_tag_id(const char *tagname,int tagtype)
{
	char query[QUERY_SIZE];
	int tmp;
	
	/* Check if tag exists */
	tmp = get_tag_id(tagname); /* Get Tag ID */
	if(tmp != KW_FAIL){
		return KW_FAIL; /* Return if tag already exists */
	}
	
	/* Check type of tag : USER / SYSTEM */
	if(tagtype == USER_TAG){
		/* Get count of existing User Tags */
		sprintf(query,"select count(tno) from TagDetails where tno>=%d;"
		             ,USER_TAG_START);
		tmp = get_field_count(query);
		
		if(tmp == KW_FAIL) { 
			return KW_FAIL;
		} else if (tmp == NO_DB_ENTRY) { /* No Entry */
			return USER_TAG_START; 
		} else { /* Increment Tag ID */
			return (tmp + USER_TAG_START);
		}
	} else { /* tagtype == SYSTEM_TAG */
		/* Get count of existing System Tags */
		sprintf(query,"select count(tno) from TagDetails where tno<%d;"
		             ,USER_TAG_START);
		tmp = get_field_count(query);
		
		if(tmp == KW_FAIL) {
			return KW_FAIL;
		} else if (tmp == NO_DB_ENTRY) { /* No Entry */
			return SYSTEM_TAG_START;
		} else { /* Increment Tag ID */
			return (tmp + 1);
		}
	}
}


/* get_field_id
 * Return id for specified field
 * 
 * @param const char *querystring 
 * @param const char *fieldname
 * @return int field_id
 * @author @SG @HP
 */
static int get_field_id(const char *querystring, const char *fieldname)
{
	sqlite3_stmt *stmt = NULL;
	char query[QUERY_SIZE];
	int status;
	
	/* Query to get field_id */
	strcpy(query, querystring);
	sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);
	sqlite3_bind_text(stmt,1, fieldname,-1,SQLITE_STATIC);
	
	status = sqlite3_step(stmt);
	if(status == SQLITE_ROW) { /* Return field_id if entry exists */
		status = atoi((const char*)sqlite3_column_text(stmt,0));
		sqlite3_finalize(stmt);
		return status;
	}
	
	sqlite3_finalize(stmt);
	return KW_FAIL;
}


/* get_file_id
 * Return id for file in kwest
 * 
 * @param char *fname - Name of File
 * @return int fno 
 * @author @SG @HP
 */
int get_file_id(const char *fname)
{
	return get_field_id("select fno from FileDetails where "
	                     "fname = :fieldname;", fname);
}


/* get_tag_id
 * Return id for a tag in kwest
 * 
 * @param char *tagname
 * @return int tno 
 * @author @SG @HP
 */
int get_tag_id(const char *tname)
{
	return get_field_id("select tno from TagDetails where "
	                     "tagname = :fieldname;", tname);
}


/* get_field_name
 * Retrieve fieldname if exists
 * 
 * @param const char *querystring 
 * @param int fieldno
 * @return char *fieldname
 * @author @SG @HP
 */
const char *get_field_name(const char *querystring, int fieldno)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;
	const char *fieldname = NULL;
	
	/* Query to get fieldname */
	sprintf(query, querystring, fieldno);
	status = sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);
	
	status = sqlite3_step(stmt);
	if(status == SQLITE_ROW){ /* return fieldname if exists */
		fieldname = strdup((const char*)sqlite3_column_text(stmt,0));
	}
	
	sqlite3_finalize(stmt);
	return fieldname;
}


/* get_file_name
 * Retrieve filename by its id
 * 
 * @param int fno - file number 
 * @return char *filename
 * @author @SG @HP
 */
const char *get_file_name(int fno)
{
	return get_field_name("select fname from FileDetails where "
	                       "fno = %d;", fno);
}


/* get_tag_name
 * Retrieve tag name by its id
 * 
 * @param int tno - tag number
 * @return char *tagname
 * @author @SG @HP
 */
const char *get_tag_name(int tno)
{
	return get_field_name("select tagname from TagDetails where "
	                       "tno = %d", tno);
}

