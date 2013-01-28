/* dbbasic.c
 * basic interation with database
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
#include <sqlite3.h>    /* SQLITE Functions */

#include "dbinit.h"
#include "dbkey.h"
#include "extract_audio_taglib.h"

#include "dbbasic.h"
#include "logging.h"
#include "flags.h"
#include "magicstrings.h"

/* ---------- Local Functions --------------------- */

/* add_metadata_file: Extract and add metadata for file in kwest
 * param: file ID, absolute path ,file name
 * return: 0 on SUCCESS
 * author: @SG
 */
static int add_metadata_file(int fno,const char *abspath,char *fname);

/* associate_file_metadata: Form association for metadata in file
 * param: metatype - Metadata Category
 * param: tagname - Metadata
 * peram: fname - File Name 
 * return: 0 on SUCCESS
 * author: @SG
 */
static int associate_file_metadata(const char *metatype,const char *tagname,
                            const char *fname);

/* ---------------- ADD/REMOVE -------------------- */

/* add_tag: Create a new user tag in kwest 
 * param: char *tagname
 * return: 0 on SUCCESS
 * author: @SG
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
	strcpy(query,"INSERT INTO TagDetails VALUES(:tno,:tagname);");
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

/* remove_tag: Remove an existing tag from kwest 
 * param: char *tagname
 * return: 0 on SUCCESS
 * author: @SG
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

/* add_file: Add file to kwest
 * param: char *abspath
 * return: 0 on SUCCESS
 * author: @SG
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
	strcpy(query,"INSERT INTO FileDetails VALUES(:fno,:fname,:abspath);");
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

/* add_metadata_file: Extract and add metadata for file in kwest
 * param: file ID, absolute path ,file name
 * return: 0 on SUCCESS
 * author: @SG
 */
static int add_metadata_file(int fno,const char *abspath,char *fname)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;
	struct metadata_audio M; 
	/*! struct metadata M; */
	void *meta;
	
	/*!
	 * meta = extract_metadata(abspath, &M);
	 * if(meta == NULL) {
		 * return KW_ERROR;
	 * }
	 */
	meta = extract_metadata_file(abspath, &M);
	if(meta == NULL) {
		extract_clear_strings(meta);
		return KW_ERROR;
	}
	
	/*! is this title,artist,album etc field necessary?
	 * how are we supposed to store metadata in an abstract way?
	 */
	strcpy(query,"INSERT INTO Audio VALUES"
	             "(:fno,:title,:artist,:album,:genre);");
	sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0); 
	
	/*! int i = 1;
	 * for(; i<=M.argc ; i++) {
		 * sqlite3_bind_text(stmt,i,M.argv[i],-1,SQLITE_STATIC);
	 * }
	 */
	sqlite3_bind_int(stmt,1,fno);
	sqlite3_bind_text(stmt,2,M.title,-1,SQLITE_STATIC);
	sqlite3_bind_text(stmt,3,M.artist,-1,SQLITE_STATIC);
	sqlite3_bind_text(stmt,4,M.album,-1,SQLITE_STATIC);
	sqlite3_bind_text(stmt,5,M.genre,-1,SQLITE_STATIC);
	
	status = sqlite3_step(stmt);
	
	if(status == SQLITE_DONE){
		associate_file_metadata(TAG_ARTIST,M.artist,fname);
		associate_file_metadata(TAG_ALBUM,M.album,fname);
		associate_file_metadata(TAG_GENRE,M.genre,fname);
	} else { /* Handle if Error while Adding Metadata */
		log_msg("add_metadata_file : %s%s",ERR_ADDING_META,fname);
	}
	extract_clear_strings(meta);
	sqlite3_finalize(stmt);
	
	return KW_SUCCESS;
}

/* associate_file_metadata: Form association for metadata in file
 * param: metatype - Metadata Category
 * param: tagname - Metadata
 * peram: fname - File Name 
 * return: 0 on SUCCESS
 * author: @SG
 */
static int associate_file_metadata(const char *metatype,const char *tagname,
                            const char *fname)
{
	char *newtag=NULL;
	
	if( (strcmp(tagname,"") == 0) || /* No meta information */ 
	    (strcmp(metatype,tagname)==0) ||
	    (strcmp(metatype,TAG_UNKNOWN)==0) ){ 
		newtag=strdup(TAG_UNKNOWN);
		newtag=strcat(strcat(newtag," "),metatype);
		/* Create Tag Unknown */
		add_tag(newtag,SYSTEM_TAG); 
		/* Associate Tag Unknown with File Type*/
		add_association(newtag,metatype,ASSOC_SUBGROUP);
		/* Tag File to Metadata Tag */
		tag_file(newtag,fname);
		free((char *)newtag);
	} else { /* Metadata Exist */
		/* Create Tag for Metadata */
		add_tag(tagname,USER_TAG);
		/* Associate Metadata tag with File Type */
		add_association(tagname,metatype,ASSOC_SUBGROUP);
		/* Tag File to Metadata Tag */
		tag_file(tagname,fname);
	}
	
	return KW_SUCCESS;
}

/* remove_file: Remove file form kwest 
 * param: char *fname - File name
 * return: 0 on SUCCESS
 * author: @SG 
 */
int remove_file(const char *fname)
{
	char query[QUERY_SIZE];
	int status;
	int fno;
	
	fno = get_file_id(fname); /* Get File ID */
	
	if(fno == KW_FAIL){ /* Return if File does not Exists */
		log_msg("remove_file : %s%s",ERR_FILE_NOT_FOUND,fname);
		return KW_ERROR;
	} 
	
	/* Remove File-Tag Associations */
	sprintf(query,"delete from FileAssociation where fno = %d;",fno);
	sqlite3_exec(get_kwdb(),query,0,0,0); 
	
	/* Remove File-metadata from Database */
	sprintf(query,"delete from Audio where fno = %d;",fno);
	sqlite3_exec(get_kwdb(),query,0,0,0);
	
	/* Remove File from Database */
	sprintf(query,"delete from FileDetails where fno = %d;",fno);
	status = sqlite3_exec(get_kwdb(),query,0,0,0); 
	
	if(status == SQLITE_OK){
		return KW_SUCCESS;
	}
	
	return KW_FAIL;
}

/* add_meta_info: Add new category to identify metadata 
 * param: filetype - Type of File
 * param: tag - Category of Metadata
 * return: 0 on SUCCESS
 * author: @SG  
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
	sprintf(query,"INSERT INTO MetaInfo VALUES('%s','%s');",filetype,tag);
	status = sqlite3_exec(get_kwdb(),query,0,0,0);
	
	if(status == SQLITE_OK){
		return KW_SUCCESS;
	}
	
	return KW_FAIL;
}

/* --------------- Tag-File Relation ------------- */

/* tag_file: Associate a tag with a file 
 * param: char *t - tagname
 * param: char *f - filename
 * return: 1 on SUCCESS
 * author: @SG
 */
int tag_file(const char *t,const char *f)
{
	sqlite3_stmt* stmt; 
	char query[QUERY_SIZE];
	int status;
	int fno,tno;
	
	fno = get_file_id(f); /* Get File ID */
	if(fno == KW_FAIL){ /* Return if File not found */
		printf("tag_file : %s%s",ERR_FILE_NOT_FOUND,f);
		return KW_ERROR;
	}
	
	tno = get_tag_id(t); /* Get Tag ID */
	if(tno == KW_FAIL){ /* Return if Tag not found */
		printf("tag_file : %s%s",ERR_TAG_NOT_FOUND,t);
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

/* untag_file: Remove the existing association between the tag and file
 * param: char *t - tagname
 * param: char *f - filename
 * return: 0 on SUCCESS
 * author: @SG
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

/* get_fname_under_tag: Return list of files associated to given tag 
 * param: char *t - tagname
 * return: sqlite3_stmt pointer
 * author: @SG
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

/* get_tags_for_file: Return list of tags associated with a given file
 * param: char *f - filename
 * return: sqlite3_stmt pointer
 * author: @SG 
 */
sqlite3_stmt *get_tags_for_file(const char *f)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;
	int fno;
	
	fno = get_file_id(f); /* Get File ID */
	if(fno == KW_FAIL){ /* Return if File not found */
		printf("get_tags_for_file : %s%s",ERR_FILE_NOT_FOUND,f);
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

/* add_association: Associate a tag with another tag 
 * param: char *t1,*t2 - tagname of both tags to be associated
 * param: int associationid - relation between tags to be formed
 * return: 0 on SUCCESS
 * author: @SG
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

/* remove_association: Remove the existing association between the two tags 
 * param: char *t1,*t2 - tagname of both tags whose associated is to be removed
 * param: int associationid - relation between tags to be removed
 * return: 0 on SUCCESS
 * author: @SG
 */
int remove_association(const char *t1,const char *t2,int associationid)
{
	char query[QUERY_SIZE];
	int status;
	int t1_id,t2_id;
	
	/* Return if relation Undefined */
	if(is_association_type(associationid) == 0){ 
		printf("remove_association : %s%d",ERR_REL_NOT_DEF,associationid);
		return KW_ERROR;
	}
	
	t1_id = get_tag_id(t1); /* Get Tag ID for tag t1*/
	if(t1_id == KW_FAIL){ /* Return if Tag not found */
		printf("remove_association : %s%s",ERR_TAG_NOT_FOUND,t1);
		return KW_ERROR;
	}
	
	t2_id = get_tag_id(t2); /* Get Tag ID for tag t2*/
	if(t2_id == KW_FAIL){ /* Return if Tag not found */
		printf("remove_association : %s%s",ERR_TAG_NOT_FOUND,t2);
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

/* get_association: Return type of association between the two tags
 * param: char *t1,*t2 - tagname of both tags in association
 * return: int associationid
 * author: @SG
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

/* get_tags_by_association: Get tags having association with another tag 
 * param: char *t - tagname
 * param: int associationid - relation between tags
 * return: sqlite3_stmt pointer
 * author: @SG 
 */
sqlite3_stmt *get_tags_by_association(const char *t,int associationid)
{
	sqlite3_stmt* stmt; 
	char query[QUERY_SIZE];
	int status;
	int t_id;
	
	/* Return if relation Undefined */
	if(is_association_type(associationid) == 0){ 
		log_msg("get_tags_by_assocn : %s%d",ERR_REL_NOT_DEF,associationid);
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

/* add_association_type: Create a new association type 
 * param: char *associationtype
 * return: int associationid
 * author: @SG 
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
		sprintf(query,"INSERT INTO Associations VALUES(%d,'%s');",
		        associationid,associationtype);
		status = sqlite3_exec(get_kwdb(),query,0,0,0);
		
		if(status == SQLITE_OK){
			return KW_SUCCESS;
		}
	}
	
	sqlite3_finalize(stmt); 
	return KW_FAIL;
}

/* is_association_type: Check if relation exists
 * param: int associationid
 * return: 1 if relation present
 * author: @SG
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

/* list_user_tags: List all tags in the system 
 * param: void
 * return: sqlite3_stmt *
 * author: @SG
 */
sqlite3_stmt *list_user_tags(void)
{
	sqlite3_stmt * stmt;
	char query[QUERY_SIZE];
	int status;
	
	/* Query to get user tags from Database */
	strcpy(query,"select tagname from TagDetails where tno>99;");
	status = sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);
	
	if(status != SQLITE_OK){ /* Error Preparing query */
		log_msg("list_user_tags : %s",ERR_PREP_QUERY);
		return NULL;
	}
	
	return stmt;
}

/* string_from_stmt: Returns data for multiple rows is query
 * param: sqlite3_stmt *stmt - statement holding query
 * return: const char* data returned by query
 * author: @SG 
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

/* istag: Check if given tag is present in system
 * param: char *t - tagname
 * return: 1 if tag present
 * author: @SG 
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

/* isfile: Check if given file is present in system
 * param: char *f - filename
 * return: 1 if file present
 * author: @SG
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

/* get_abspath_by_fname: return absolute path of file
 * param: char *path - kwest path
 * return: char * absolute path
 * author: @SG 
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

/* rename_file: rename file existing in kwest
 * param: char *from - existing name of file
 * param: char *to - new name of file
 * return: KW_SUCCESS, KW_ERROR, KW_FAIL
 * author: @SG 
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
