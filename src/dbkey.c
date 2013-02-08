/**
 * @file dbkey.c
 * @brief handle requests for files and tags
 * @author Sahil Gupta
 * @date December 2012
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
#include <sqlite3.h>

#include "dbkey.h"
#include "dbinit.h"
#include "flags.h"

/**
 * @brief Return count of entries in database
 * @param querystring 
 * @return count
 * @author SG
 * @todo return id of deleted entry if available
 */
static int get_field_count(const char *querystring)
{
	sqlite3_stmt *stmt = NULL;
	char query[QUERY_SIZE];
	int status;
	char *tmp;
	
	/* Query to get field count */
	strcpy(query, querystring);
	sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);
	
	status = sqlite3_step(stmt);
	if(status == SQLITE_ROW) { /* Return field count */
		tmp = (const char*)sqlite3_column_text(stmt,0);
		if(tmp == NULL){
			sqlite3_finalize(stmt);
			return NO_DB_ENTRY;
		}
		status = atoi(tmp);
		sqlite3_finalize(stmt);
		return status;
	}
	
	sqlite3_finalize(stmt);
	return KW_FAIL;
}

/**
 * @brief Return id for specified field
 * @param querystring 
 * @param fieldname
 * @return field_id
 * @author SG HP
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

/**
 * @brief Retrieve fieldname if exists
 * @param querystring 
 * @param fieldno
 * @return fieldname
 * @author SG HP
 */
static const char *get_field_name(const char *querystring, int fieldno)
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

/**
 * @brief Generate id for new file to be added in kwest
 * @param abspath - Absolute Path of File
 * @return fno
 * @author SG 
 */
int set_file_id(const char *abspath)
{
	int tmp = -1;
	
	/* Check if file exists */
	tmp = get_file_id_by_abspath(abspath); /* Get File ID */
	if(tmp != KW_FAIL){ 
		return KW_FAIL; /* Return if file already exists */
	}
	
	/* Query to get maximum no of files existing in database */
	tmp = get_field_count("select max(fno) from FileDetails;");
	
	if(tmp == KW_FAIL) {
		return KW_FAIL;
	} else if(tmp == NO_DB_ENTRY) { /* No Entry */
		return FILE_START;
	} else { /* Increment File ID */
		return tmp + 1;
	}
}

/**
 * @brief Generate id for new tag to be created in kwest
 * @param tagname
 * @param tagtype
 * @return tno 
 * @author SG 
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

/**
 * @brief Return id for file in kwest
 * @param abspath
 * @return fno 
 * @author SG
 */
int get_file_id_by_abspath(const char *abspath)
{
	return get_field_id("select fno from FileDetails where "
	                    "abspath = :fieldname;", abspath);
}

/**
 * @brief Return id for file in kwest
 * @param path Kwest path containing tags
 * @return fno 
 * @author SG
 */
int get_file_id_by_intersection(const char *path)
{
	char query[QUERY_SIZE];
	char *tmp_ptr,*tmp_path;
	char *tag;
	char tagid[5];
	
	tmp_path = strdup(path);
	strcpy(query,"");
	
	while( (tmp_ptr = strrchr(tmp_path,'/')) != NULL)
	{
		tag = tmp_ptr + 1;
		*tmp_ptr = '\0';
		
		sprintf(tagid,"%d",get_tag_id(tag));
		strcat(query,"select fno from FileAssociation where tno=");
		strcat(query,tagid);
		if((tmp_ptr = strrchr(tmp_path,'/')) != NULL) {
			strcat(query," INTERSECT ");
		} else {
			break;
		}
	}
	free((char*)tmp_path);
	
	return 0;
}

/**
 * @brief Return id for file in kwest
 * @param path Path containing tagname and filename
 * @return fno 
 * @author SG HP
 */
int get_file_id_by_tag(const char *path)
{
	char query[QUERY_SIZE];
	char *filename = strrchr(path, '/') + 1;
	char *tptr = filename -2;
	char *tagname = NULL;
	char *t2 = NULL;
	int tno;
	
	while(*tptr != '/') tptr--; /* seperat directory name */
	tptr++;
	
	tagname = malloc(filename - tptr);
	t2 = tagname;
	while(*tptr != '/') {
		*t2 = *tptr;
		t2++; tptr++;
	}
	*t2 = '\0';
	
	tno=get_tag_id(tagname);
	sprintf(query,"select fno from FileAssociation where tno = %d INTERSECT"
	        " select fno from FileDetails where fname = :fieldname;",tno);
	
	return get_field_id(query, filename);
}

/**
 * @brief Return id for a tag in kwest
 * @param tname - tagname
 * @return tno 
 * @author SG HP
 */
int get_tag_id(const char *tname)
{
	return get_field_id("select tno from TagDetails where "
	                    "tagname = :fieldname;", tname);
}

/**
 * @brief Retrieve filename by its id
 * @param fno - file number 
 * @return filename
 * @author SG HP
 */
const char *get_file_name(int fno)
{
	return get_field_name("select fname from FileDetails where "
	                      "fno = %d;", fno);
}

/**
 * @brief Retrieve tag name by its id
 * @param tno - tag number
 * @return tagname
 * @author SG HP
 */
const char *get_tag_name(int tno)
{
	return get_field_name("select tagname from TagDetails where "
	                      "tno = %d", tno);
}
