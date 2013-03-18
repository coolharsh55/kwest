/**
 * @file dbbasic.c
 * @brief basic interation with database
 * @author Sahil Gupta
 * @date March 2013
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

#include "dbbasic.h"
#include "dbinit.h"
#include "dbkey.h"
#include "apriori.h"
#include "logging.h"
#include "flags.h"
#include "magicstrings.h"

#include "metadata_extract.h"
#include "plugins_extraction.h"

/* ---------- Local Functions --------------------- */

/*
 * Extract and add metadata for file in kwest
 */
static int add_metadata_file(int fno,const char *abspath,char *fname);

/* ---------------- ADD/REMOVE -------------------- */

/**
 * @fn int add_tag(const char *tagname,int tagtype)
 * @brief Create a new user tag in kwest
 * @param tagname name of the tag
 * @param tagtype systemtag / usertag
 * @return KW_SUCCESS on SUCCESS
 * @return KW_FAIL on FAIL
 * @return KW_ERROR on ERROR
 * @author SG
 * @see add_association
 */
int add_tag(const char *tagname,int tagtype)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;
	int tno; /* Tag ID */

	/* Call Function to set tno for Tag */
	if(tagtype == USER_TAG){
		tno = set_tag_id(tagname,USER_TAG); /* Add User Tag */
	} else if(tagtype == SYSTEM_TAG) {
		tno = set_tag_id(tagname,SYSTEM_TAG); /* Add System Tag */
	}

	/* Return if Tag Exists */
	if(tno == KW_FAIL){
		return KW_ERROR;
	}

	/* Insert (tno, tagname) in TagDetails Table */
	strcpy(query,"insert into TagDetails values(:tno,:tagname);");
	sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	sqlite3_bind_int(stmt,1,tno);
	sqlite3_bind_text(stmt,2,tagname,-1,SQLITE_STATIC);

	status = sqlite3_step(stmt);
	if(status == SQLITE_DONE){
		sqlite3_finalize(stmt);
		return KW_SUCCESS;
	}

	sqlite3_finalize(stmt);
	return KW_FAIL;
}

/**
 * @brief Remove an existing tag from kwest
 * @param tagname
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author SG
 */
int remove_tag(const char *tagname)
{
	char query[QUERY_SIZE];
	int status;
	int tno;

	tno = get_tag_id(tagname); /* Get Tag ID */

	/* Return if Tag does not Exists */
	if(tno == KW_FAIL){
		log_msg("remove_tag : %s%s",ERR_TAG_NOT_FOUND,tagname);
		return KW_ERROR;
	}

	/* Remove all Tag-Tag Associations */
	sprintf(query,"delete from TagAssociation where t1 = %d or t2 = %d;",
	        tno,tno);
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	/* Remove all File-Tag Associations */
	sprintf(query,"delete from FileAssociation where tno = %d;",tno);
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	/* Remove all Tag from database */
	sprintf(query,"delete from TagDetails where tno = %d;",tno);
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	if(status == SQLITE_OK){
		return KW_SUCCESS;
	}

	return KW_FAIL;
}

/**
 * @brief Add file to kwest
 * @param abspath
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author SG
 */
int add_file(const char *abspath)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;
	int fno;
	char *fname;

	fno = set_file_id(abspath); /* Call Function to set fno for File */
	fname = strrchr(abspath,'/')+1;

	if(fno == KW_FAIL){ /* Return if File already Exists */
		return KW_ERROR;
	}

	/* Query : Insert (fno, fname, abspath) in FileDetails Table */
	strcpy(query,"insert into FileDetails values(:fno,:fname,:abspath);");
	sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	sqlite3_bind_int(stmt,1,fno);
	sqlite3_bind_text(stmt,2,fname,-1,SQLITE_STATIC);
	sqlite3_bind_text(stmt,3,abspath,-1,SQLITE_STATIC);

	status = sqlite3_step(stmt);
	if(status != SQLITE_DONE){
		log_msg("add_file : %s%s",ERR_ADDING_FILE,fname);
		sqlite3_finalize(stmt);
		return KW_FAIL;
	}

	sqlite3_finalize(stmt);
	/* Get Metadata for file */
	add_metadata_file(fno,abspath,fname);

	if(status == SQLITE_DONE){
		return KW_SUCCESS;
	}

	return KW_FAIL;
}

/**
 * @brief Store metadata for file in database
 * @param int fno - file id
 * @param kw_metadata Structure containing metadata
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL
 * @author SG HP
 */
static int insert_metadata_file(int fno, struct kw_metadata *kw_M)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;
	int i;

	strcpy(query, "insert into ");
	strcat(query, kw_M->type);
	strcat(query, " values(:fno");
	for (i = 0 ; i < kw_M->tagc ; i++) {
		strcat(query, ",:");
		strcat(query, kw_M->tagtype[i]);
	}
	strcat(query, ");");
	sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	sqlite3_bind_int(stmt,1,fno);
	for(i = 2; i <= (kw_M->tagc + 1) ; i++) {
		sqlite3_bind_text(stmt, i, kw_M->tagv[i-2], -1, SQLITE_STATIC);
	}

	status = sqlite3_step(stmt);

	if(status == SQLITE_DONE){
		sqlite3_finalize(stmt);
		return KW_SUCCESS;
	}

	sqlite3_finalize(stmt);
	return KW_FAIL;
}

/**
 * @brief Extract and add metadata for file in kwest
 * @param int fno - file id
 * @param abspath - absolute path
 * @param fname - file name
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author SG HP
 */
static int add_metadata_file(int fno,const char *abspath,char *fname)
{
	int status;
	struct kw_metadata kw_M;

	status = metadata_extract(abspath, &kw_M);
	if(status == KW_FAIL) {
		return KW_ERROR;
	}

	/*!
	status = insert_metadata_file(fno, &kw_M);

	if(status != KW_SUCCESS) {
		log_msg("add_metadata_file : %s%s",ERR_ADDING_META,fname);
		kw_M.do_cleanup(&kw_M);
		return KW_FAIL;
	}
	*/

	kw_M.obj = (void *)fname;
	kw_M.do_cleanup(&kw_M);

	return KW_SUCCESS;
}

/**
 * @brief Remove file form kwest
 * @param abspath Absolute path of file
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author SG
 */
int remove_file(const char *abspath)
{
	char query[QUERY_SIZE];
	int status;
	int fno;

	fno = get_file_id(strrchr(abspath,'/') + 1); /* Get File ID */

	if(fno == KW_FAIL){ /* Return if File does not Exists */
		log_msg("remove_file : %s%s",ERR_FILE_NOT_FOUND,abspath);
		return KW_ERROR;
	}

	/* Remove File-Tag Associations */
	sprintf(query,"delete from FileAssociation where fno = %d;",fno);
	sqlite3_exec(get_kwdb(),query,0,0,0);

	/** @todo Generalize structure to remove file medatata */
	/* Remove File-metadata from Database */
	/* sprintf(query,"delete from Audio where fno = %d;",fno);
	sqlite3_exec(get_kwdb(),query,0,0,0); */

	/* Remove File from Database */
	sprintf(query,"delete from FileDetails where fno = %d;",fno);
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	if(status == SQLITE_OK){
		return KW_SUCCESS;
	}

	return KW_FAIL;
}

/**
 * @brief Add new category to identify metadata
 * @param filetype - Type of File
 * @param tag - Category of Metadata
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author SG
 */
int add_meta_info(const char *filetype,const char *tag)
{
	sqlite3_stmt* stmt;
	char query[QUERY_SIZE];
	int status;

	/* Check if info already exists */
	sprintf(query,"select count(*) from MetaInfo where "
	              "filetype = '%s' and tag = '%s';",filetype,tag);
	sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	status = sqlite3_step(stmt);
	if(status == SQLITE_ROW) {
		if(atoi((const char*)sqlite3_column_text(stmt,0))>0) {
			sqlite3_finalize(stmt);
			return KW_ERROR;
		}
	}
	sqlite3_finalize(stmt);

	/* Query to add metainfo */
	sprintf(query,"insert into MetaInfo values('%s','%s');",filetype,tag);
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	if(status == SQLITE_OK){
		return KW_SUCCESS;
	}

	return KW_FAIL;
}

/* --------------- Tag-File Relation ------------- */

/**
 * @brief Associate a tag with a file
 * @param t - tagname
 * @param f - filename
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author SG
 */
int tag_file(const char *t,const char *f)
{
	sqlite3_stmt* stmt;
	char query[QUERY_SIZE];
	int status;
	int fno,tno;

	fno = get_file_id(f); /* Get File ID */
	if(fno == KW_FAIL){ /* Return if File not found */
		log_msg("tag_file : %s%s",ERR_FILE_NOT_FOUND,f);
		return KW_ERROR;
	}

	tno = get_tag_id(t); /* Get Tag ID */
	if(tno == KW_FAIL){ /* Return if Tag not found */
		log_msg("tag_file : %s%s",ERR_TAG_NOT_FOUND,t);
		return KW_ERROR;
	}

	/* Query : check if entry exists in File Association Table */
	sprintf(query,"select fno from FileAssociation where tno = %d;",tno);
	status = sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	do { /* Query return all files under tag t */
		status = sqlite3_step(stmt);

		if(status == SQLITE_ROW) { /* check if f is in tag t */
		      if(fno == atoi((const char*)sqlite3_column_text(stmt,0))){
			      sqlite3_finalize(stmt);
			      return KW_ERROR; /* File is already tagged */
		      }
		}
	}while(status == SQLITE_ROW);
	sqlite3_finalize(stmt);

	/* Query : add tno,fno to File Association Table */
	if(status == SQLITE_DONE) { /* File not yet tagged */
		sprintf(query,"insert into FileAssociation values(%d,%d);"
		             ,tno,fno);
		status = sqlite3_exec(get_kwdb(),query,0,0,0);

		if(status == SQLITE_OK){
			return KW_SUCCESS;
		}
	}

	return KW_FAIL;
}

/**
 * @brief Remove the existing association between the tag and file
 * @param t - tagname
 * @param f - filename
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author SG
 */
int untag_file(const char *t,const char *f)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;
	int fno,tno;

	log_msg("untag file: %s :: %s", t, f);

	fno = get_file_id(f); /* Get File ID */
	if(fno == KW_FAIL){ /* Return if File not found */
		log_msg("untag_file : %s%s",ERR_FILE_NOT_FOUND,f);
		return KW_ERROR;
	}

	tno = get_tag_id(t); /* Get Tag ID */
	if(tno == KW_FAIL){ /* Return if Tag not found */
		log_msg("untag_file : %s%s",ERR_TAG_NOT_FOUND,t);
		return KW_ERROR;
	}

	/* Query to remove File-Tag Association */
	sprintf(query,"delete from FileAssociation where fno = %d and tno = %d;"
	             ,fno,tno);
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	if(status != SQLITE_OK){
		log_msg("untag operation failed");
		return KW_FAIL;
	}
	log_msg("untag operation success");

	/* Remove file if not under any tag */
	sprintf(query,"select count(*) from FileAssociation where fno = %d;"
	             ,fno);
	sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	status = sqlite3_step(stmt);
	if(status == SQLITE_ROW) {
		if(atoi((const char*)sqlite3_column_text(stmt,0)) == 0) {
			sqlite3_finalize(stmt);
			status = remove_file(f);
			if(status == KW_SUCCESS){
				log_msg("removing file from database");
				return KW_SUCCESS;
			}
		}
	}

	sqlite3_finalize(stmt);
	return KW_SUCCESS;
}

/**
 * @brief Return list of files associated to given tag
 * @param t - tagname
 * @return sqlite3_stmt pointer : SUCCESS, NULL : FAIL
 * @author SG
 */
sqlite3_stmt *get_fname_under_tag(const char *t)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;
	int tno;

	tno = get_tag_id(t); /* Get Tag ID */
	if(tno == KW_FAIL){ /* Return if Tag not found */
		log_msg("get_fname_under_tag : %s%s",ERR_TAG_NOT_FOUND,t);
		return NULL;
	}

	/* Query to get all files associated with tag t */
	sprintf(query,"select fname from FileDetails where fno in"
	              "(select fno from FileAssociation where tno = %d);",tno);
	status = sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	if(status != SQLITE_OK){ /* Error Preparing query */
		log_msg("get_fname_under_tag : %s",ERR_PREP_QUERY);
		return NULL;
	}

	return stmt;
}

/**
 * @brief Return list of file id associated to given tag
 * @param t - tagname
 * @return sqlite3_stmt pointer : SUCCESS, NULL : FAIL
 * @author SG
 */
sqlite3_stmt *get_fid_under_tag(const char *t)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;
	int tno;

	tno = get_tag_id(t); /* Get Tag ID */
	if(tno == KW_FAIL){ /* Return if Tag not found */
		log_msg("get_fname_under_tag : %s%s",ERR_TAG_NOT_FOUND,t);
		return NULL;
	}

	/* Query to get all files associated with tag t */
	sprintf(query,"select fno from FileAssociation where tno = %d;",tno);
	status = sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	if(status != SQLITE_OK){ /* Error Preparing query */
		log_msg("get_fid_under_tag : %s",ERR_PREP_QUERY);
		return NULL;
	}

	return stmt;
}

/**
 * @brief Return list of tag id associated to given tag
 * @param t - tagname
 * @return sqlite3_stmt pointer : SUCCESS, NULL : FAIL
 * @author SG
 */
sqlite3_stmt *get_tid_under_tag(const char *t)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;
	int tno;

	tno = get_tag_id(t); /* Get Tag ID */
	if(tno == KW_FAIL){ /* Return if Tag not found */
		log_msg("get_tid_under_tag : %s%s",ERR_TAG_NOT_FOUND,t);
		return NULL;
	}

	/* Query to get all files associated with tag t */
	sprintf(query,"select distinct t1 from TagAssociation where t2 = %d;",
	        tno);
	status = sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	if(status != SQLITE_OK){ /* Error Preparing query */
		log_msg("get_tid_under_tag : %s",ERR_PREP_QUERY);
		return NULL;
	}

	return stmt;
}

/**
 * @brief Return list of tags associated with a given file
 * @param f - filename
 * @return sqlite3_stmt pointer : SUCCESS, NULL : FAIL
 * @author SG
 */
sqlite3_stmt *get_tags_for_file(const char *f)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;
	int fno;

	fno = get_file_id(f); /* Get File ID */
	if(fno == KW_FAIL){ /* Return if File not found */
		log_msg("get_tags_for_file : %s%s",ERR_FILE_NOT_FOUND,f);
		return NULL;
	}

	/* Query to get all tags associated with file f */
	sprintf(query,"select tagname from TagDetails where tno in"
	        "(select tno from FileAssociation where fno = %d);",fno);
	status = sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	if(status != SQLITE_OK){ /* Error Preparing query */
		log_msg("get_tags_for_file : %s",ERR_PREP_QUERY);
		return NULL;
	}

	return stmt;
}


/*----------------- Tag-Tag Relation ------------------*/

/**
 * @brief Associate a tag with another tag
 * @param t1,t2 - tagname of both tags to be associated
 * @param associationid - relation between tags to be formed
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author SG
 */
int add_association(const char *t1,const char *t2,int associationid)
{
	char query[QUERY_SIZE];
	int status;
	int t1_id,t2_id;

	/* Return if relation Undefined */
	if(is_association_type(associationid) == 0){
		log_msg("add_association : %s%d",ERR_REL_NOT_DEF,associationid);
		return KW_ERROR;
	}

	t1_id = get_tag_id(t1); /* Get Tag ID for tag t1*/
	if(t1_id == KW_FAIL){ /* Return if Tag not found */
		log_msg("add_association : %s%s",ERR_TAG_NOT_FOUND,t1);
		return KW_ERROR;
	}

	t2_id = get_tag_id(t2); /* Get Tag ID for tag t2*/
	if(t2_id == KW_FAIL){ /* Return if Tag not found */
		log_msg("add_association : %s%s",ERR_TAG_NOT_FOUND,t2);
		return KW_ERROR;
	}

	/* Query : check if entry already exists in TagAssociation Table */
	if(get_association(t1,t2)!= KW_FAIL){
		return KW_ERROR;
	}

	/* Query : add (t1, t2, associationtype) to TagAssociation Table */
	sprintf(query,"insert into TagAssociation values(%d,%d,%d);",
	        t1_id,t2_id,associationid);
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	if(status == SQLITE_OK){
		return KW_SUCCESS;
	}

	return KW_FAIL;
}

/**
 * @brief Remove the existing association between the two tags
 * @param t1,t2 - tagname of both tags whose associated is to be removed
 * @param associationid - relation between tags to be removed
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author SG
 */
int remove_association(const char *t1,const char *t2,int associationid)
{
	char query[QUERY_SIZE];
	int status;
	int t1_id,t2_id;

	/* Return if relation Undefined */
	if(is_association_type(associationid) == 0){
		log_msg("remove_association : %s%d",ERR_REL_NOT_DEF,
		       associationid);
		return KW_ERROR;
	}

	t1_id = get_tag_id(t1); /* Get Tag ID for tag t1*/
	if(t1_id == KW_FAIL){ /* Return if Tag not found */
		log_msg("remove_association : %s%s",ERR_TAG_NOT_FOUND,t1);
		return KW_ERROR;
	}

	t2_id = get_tag_id(t2); /* Get Tag ID for tag t2*/
	if(t2_id == KW_FAIL){ /* Return if Tag not found */
		log_msg("remove_association : %s%s",ERR_TAG_NOT_FOUND,t2);
		return KW_ERROR;
	}

	/* Query to remove association between t1 and t2 */
	sprintf(query,"delete from TagAssociation where "
	              "t1 = %d and t2 = %d and associationid = %d;",
	              t1_id,t2_id,associationid);
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	if(status == SQLITE_OK){
		return KW_SUCCESS;
	}

	return KW_FAIL;
}

/**
 * @brief Return type of association between the two tags
 * @param t1,t2 - tagname of both tags in association
 * @return associationid : SUCCESS, KW_FAIL : FAIL, KW_ERROR : ERROR
 * @author SG
 */
int get_association(const char *t1,const char *t2)
{
	sqlite3_stmt* stmt;
	char query[QUERY_SIZE];
	int status;
	int t1_id,t2_id,associationid;

	t1_id = get_tag_id(t1); /* Get Tag ID for tag t1*/
	if(t1_id == KW_FAIL){ /* Return if Tag not found */
		log_msg("get_association : %s%s",ERR_TAG_NOT_FOUND,t1);
		return KW_ERROR;
	}

	t2_id = get_tag_id(t2); /* Get Tag ID for tag t2*/
	if(t2_id == KW_FAIL){ /* Return if Tag not found */
		log_msg("get_association : %s%s",ERR_TAG_NOT_FOUND,t2);
		return KW_ERROR;
	}

	/* Query to get association between t1 and t2 */
	sprintf(query,"select associationid from TagAssociation where "
	              "t1 = %d and t2 = %d;",t1_id,t2_id);
	status = sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	status = sqlite3_step(stmt);
	if(status == SQLITE_ROW) {
		associationid = atoi((const char*)sqlite3_column_text(stmt,0));
		sqlite3_finalize(stmt);
		return associationid; /* return associationid */
	}

	sqlite3_finalize(stmt);
	return KW_FAIL; /*No association between tags*/
}

/**
 * @brief Get tags having association with another tag
 * @param t - tagname
 * @param associationid - relation between tags
 * @return sqlite3_stmt pointer: SUCCESS, NULL : FAIL
 * @author SG
 */
sqlite3_stmt *get_tags_by_association(const char *t,int associationid)
{
	sqlite3_stmt* stmt;
	char query[QUERY_SIZE];
	int status;
	int t_id;

	/* Return if relation Undefined */
	if(is_association_type(associationid) == 0){
		log_msg("get_tags_by_assocn : %s%d",ERR_REL_NOT_DEF,
		        associationid);
		return NULL;
	}

	t_id = get_tag_id(t); /* Get Tag ID */
	if(t_id == KW_FAIL){ /* Return if Tag not found */
		log_msg("get_tags_by_assocn : %s%s",ERR_TAG_NOT_FOUND,t);
		return NULL;
	}

	/* Query to get tags associated with tag t */
	sprintf(query,"select tagname from TagDetails where tno in"
	              "(select t1 from TagAssociation where "
	              "t2 = %d and associationid = %d);",
	              t_id,associationid);
	status = sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	if(status != SQLITE_OK){ /* Error Preparing query */
		return NULL;
	}

	return stmt;
}

/* ----------------- Associations --------------------- */

/**
 * @brief Create a new association type
 * @param associationtype
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author SG
 */
int add_association_type(const char *associationtype)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;
	int associationid = 0;

	/* Query : check if entry exists in Associations Table */
	sprintf(query,"select count(associationid) from Associations where "
	              "associationtype = '%s';",associationtype);
	status = sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	status = sqlite3_step(stmt);
	if(status == SQLITE_ROW) {
		if(atoi((const char*)sqlite3_column_text(stmt,0))!= 0) {
			sqlite3_finalize(stmt);
			return KW_ERROR; /* Relation Exists */
		}
	}
	sqlite3_finalize(stmt);
	stmt = NULL;

	/* Query to get maximum relation id existing in database */
	strcpy(query,"select max(associationid) from Associations;");
	status = sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	status = sqlite3_step(stmt);
	if(status == SQLITE_ROW) {
		if((const char*)sqlite3_column_text(stmt,0) == NULL){
			associationid = 0; /* First Entry */
		} else {
			associationid =
			    atoi((const char*)sqlite3_column_text(stmt,0))+1;
		}
		sqlite3_finalize(stmt);

		/* Query:add (assocnid,assocntype) to Association Table */
		sprintf(query,"insert into Associations values(%d,'%s');",
		        associationid,associationtype);
		status = sqlite3_exec(get_kwdb(),query,0,0,0);

		if(status == SQLITE_OK){
			return KW_SUCCESS;
		}
	}

	sqlite3_finalize(stmt);
	return KW_FAIL;
}

/**
 * @brief Check if relation exists
 * @param associationid
 * @return true if relation present, KW_FAIL : FAIL
 * @author SG
 */
int is_association_type(int associationid)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;

	sprintf(query,"select count(associationid) from Association where "
	              "associationid = %d;",associationid);
	status = sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	status = sqlite3_step(stmt);
	if(status == SQLITE_ROW) {
		associationid = atoi((const char*)sqlite3_column_text(stmt,0));
		sqlite3_finalize(stmt);
		return associationid;
	}

	sqlite3_finalize(stmt);
	return KW_FAIL;

}

/* --------------------- Others --------------------- */

/**
 * @brief Get all tags in kwest
 * @param void
 * @return sqlite3_stmt pointer: SUCCESS, NULL : FAIL
 * @author SG
 */
sqlite3_stmt *get_all_tno(void)
{
	sqlite3_stmt * stmt;
	char query[QUERY_SIZE];
	int status;

	/* Query to get user tags from Database */
	strcpy(query,"select tno from TagDetails;");
	status = sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	if(status != SQLITE_OK){ /* Error Preparing query */
		log_msg("list_user_tags : %s",ERR_PREP_QUERY);
		return NULL;
	}

	return stmt;
}

/**
 * @brief Returns data for multiple rows in query
 * @param stmt - statement holding query
 * @return data returned by query : SUCCESS, NULL : FAIL
 * @author SG
 */
const char* string_from_stmt(sqlite3_stmt *stmt)
{
	int status;

	if(stmt == NULL) {
		log_msg("string_from_stmt: NULL");
		return NULL;
	}

	status = sqlite3_step(stmt); /* Execute Query */
	if(status == SQLITE_ROW){ /* Return data if present */
		return (const char*)sqlite3_column_text(stmt,0);
	} else { /* Return NULL to mark end of Data */
		sqlite3_finalize(stmt);
		return NULL;
	}
}

/* -------------------- Fuse Functions --------------------- */

/**
 * @brief Check if given tag is present in system
 * @param t - tagname
 * @return true if tag present, KW_FAIL : FAIL
 * @author SG
 */
bool istag(const char *t)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;
	int tmp;  /* To Hold result of Query */

	/* check if tag with name t exist */
	sprintf(query,"select count(*) from TagDetails where tagname = :t;");
	sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);
	sqlite3_bind_text(stmt,1,t,-1,SQLITE_STATIC);

	status = sqlite3_step(stmt);
	if(status == SQLITE_ROW) {
		tmp = atoi((const char*)sqlite3_column_text(stmt,0));
		sqlite3_finalize(stmt);
		return tmp;
	}

	sqlite3_finalize(stmt);
	return KW_FAIL;
}

/**
 * @brief Check if given file is present in system
 * @param f - filename
 * @return true if file present, KW_FAIL : FAIL
 * @author SG
 */
bool isfile(const char *f)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;
	int tmp; /* To Hold result of Query */

	/* check if file with name f exist */
	strcpy(query,"select count(*) from FileDetails where fname = :f;");
	sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);
	sqlite3_bind_text(stmt,1,f,-1,SQLITE_STATIC);

	status = sqlite3_step(stmt);
	if(status == SQLITE_ROW) {
		tmp = atoi((const char*)sqlite3_column_text(stmt,0));
		sqlite3_finalize(stmt);
		return tmp;
	}

	sqlite3_finalize(stmt);
	return KW_FAIL;
}

/**
 * @brief Return absolute path of file
 * @param fname - file name
 * @return absolute path : SUCCESS, NULL : FAIL
 * @author SG
 */
char *get_abspath_by_fname(const char *fname)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;
	char *abspath;

	/* Query to get absolute path from file name */
	strcpy(query,"select abspath from FileDetails where fname = :fname;");
	sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);
	sqlite3_bind_text(stmt,1,fname,-1,SQLITE_STATIC);

	status = sqlite3_step(stmt);
	if(status == SQLITE_ROW) {
		abspath = strdup((const char*)sqlite3_column_text(stmt,0));
		sqlite3_finalize(stmt);
		return abspath;
	}

	sqlite3_finalize(stmt);
	return NULL;
}

/**
 * @brief Rename file existing in kwest
 * @param from - existing name of file
 * @param to - new name of file
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author SG
 */
int rename_file(const char *from, const char *to)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;
	int fno;

	log_msg("rename_file: %s :: %s", from, to);

	fno = get_file_id(from); /* Get File ID */

	if(fno == KW_FAIL){ /* Return if File does not Exists */
		log_msg("rename_file : %s%s",ERR_FILE_NOT_FOUND,from);
		return KW_ERROR;
	}

	strcpy(query,"update FileDetails set fname=:from where fno=:fno;");
	sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	sqlite3_bind_text(stmt,1,from,-1,SQLITE_STATIC);
	sqlite3_bind_int(stmt,2,fno);

	status = sqlite3_step(stmt);
	if(status == SQLITE_DONE){
		log_msg("rename operation successful");
		sqlite3_finalize(stmt);
		return KW_SUCCESS;
	}
	sqlite3_finalize(stmt);

	log_msg("rename_file : %s%s",ERR_RENAMING_FILE,from);
	return KW_FAIL;
}
