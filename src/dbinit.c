/* dbinit.c
 * create and initialize database
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
#include <pwd.h>        /* Function : getpwuid */
#include <unistd.h>     /* Function : getuid */
#include <errno.h>
#include <sys/stat.h>

#include "dbbasic.h"

#include "dbinit.h"
#include "flags.h"
#include "magicstrings.h"
#include "logging.h"


/* ------------ Database Connections ---------------- */

/* get_kwdb: Initialize Return sqlite* Object
 * return: sqlite3 pointer
 * author: @SG
 */
sqlite3 *get_kwdb()
{
	static sqlite3 *db = NULL;
	
	if(db == NULL){
		int status;
		struct passwd *pw;
		const char *homedir;
		char kwestdir[QUERY_SIZE];
		
		/* Set path for database file to /home/user/.config */
		pw = getpwuid(getuid());
		homedir = pw->pw_dir; /* initial working directory */
		strcpy(kwestdir,strcat((char *)homedir,CONFIG_LOCATION));
		
		if(mkdir(kwestdir, KW_STDIR) == -1 && errno != EEXIST) {
			return NULL;
		} 
		
		strcat(kwestdir, DATABASE_NAME);
		status = sqlite3_open(kwestdir,&db);
		
		if(status != SQLITE_OK) {
			printf("DB INIT FAIL!!!");
			log_msg("%s",ERR_DB_CONN);
			return NULL;
		}
	}
	
	return db;
}

/* create_db: Create Kwest database for first use
 * return: 0 on SUCCESS
 * author: @SG
 */
int create_db(void)
{
	char query[QUERY_SIZE];
	int status;

	strcpy(query,"CREATE TABLE IF NOT EXISTS FileDetails "
	"(fno INTEGER PRIMARY KEY,fname VARCHAR2(50),abspath VARCHAR2(100));");
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	strcpy(query,"CREATE TABLE IF NOT EXISTS TagDetails "
	"(tno INTEGER PRIMARY KEY,tagname VARCHAR2(50));");
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	strcpy(query,"CREATE TABLE IF NOT EXISTS FileAssociation "
	"(tno INTEGER,fno INTEGER);");
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	strcpy(query,"CREATE TABLE IF NOT EXISTS TagAssociation "
	"(t1 INTEGER,t2 INTEGER,associationid INTEGER);");
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	strcpy(query,"CREATE TABLE IF NOT EXISTS MetaInfo "
	"(filetype VARCHAR2(20),tag VARCHAR2(20));");
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	strcpy(query,"CREATE TABLE IF NOT EXISTS Audio "
	"(fno INTEGER,title VARCHAR2(75),artist VARCHAR2(75),"
	"album VARCHAR2(75),genre VARCHAR2(75));");
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	strcpy(query,"CREATE TABLE IF NOT EXISTS Associations "
	"(associationid INTEGER PRIMARY KEY,associationtype VARCHAR2(20));");
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	/* Possible Tag-Tag Relations */
	add_association_type(ASSOC_SYSTEM);
	add_association_type(ASSOC_PROBAB);
	add_association_type(ASSOC_SUBGRP);
	add_association_type(ASSOC_RELATD);
	add_association_type(ASSOC_NOTREL);

	add_tag(TAG_ROOT, SYSTEM_TAG);
	add_tag(TAG_FILES, SYSTEM_TAG);
	add_association(TAG_FILES, TAG_ROOT, ASSOC_SUBGROUP);
	
	return status;
}

/* close_db: Close Kwest Database Connection
 * return: 0 on SUCCESS
 * author: @SG
 */
int close_db(void)
{
	int status;

	status = sqlite3_close(get_kwdb());

	if (status != SQLITE_OK) {  
		log_msg("%s", ERR_DB_CLOSE);
	}

	return status;
}

/* begin_transaction: 
 * return: 0 on SUCCESS
 * author: @SG
 */
int begin_transaction()
{
	return (int)sqlite3_exec(get_kwdb(),"BEGIN",0,0,0);
}

/* commit_transaction: 
 * return: 0 on SUCCESS
 * author: @SG
 */
int commit_transaction()
{	
	return (int)sqlite3_exec(get_kwdb(),"COMMIT",0,0,0); 
}
