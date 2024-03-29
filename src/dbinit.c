/**
 * @file dbinit.c
 * @brief create and initialize database
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
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <pwd.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "dbinit.h"
#include "dbbasic.h"
#include "logging.h"
#include "flags.h"
#include "magicstrings.h"


/**
 * @brief Get users home directory absolute path
 * @param homedir
 * @return void
 * @author SG
 */
void get_homedir(char **homedir)
{
	struct passwd *pw;

	pw = getpwuid(getuid());
	*homedir = pw->pw_dir;
}

/**
 * @brief Initialize Return sqlite pointer object
 * @param void
 * @return sqlite3 pointer : SUCCESS, NULL : FAIL
 * @author SG
 */
sqlite3 *get_kwdb(void)
{
	static sqlite3 *db = NULL;

	if(db == NULL){
		int status;
		char *homedir;
		char kwestdir[QUERY_SIZE];

		/* Set path for database file to /home/user/.config */
		get_homedir(&homedir);
		strcpy(kwestdir,strcat(homedir,CONFIG_LOCATION));

		if(mkdir(kwestdir, KW_STDIR) == -1 && errno != EEXIST) {
			return NULL;
		}

		strcat(kwestdir, DATABASE_NAME);
		status = sqlite3_open(kwestdir,&db);

		if(status != SQLITE_OK) {
			log_msg("%s",ERR_DB_CONN);
			return NULL;
		}
	}

	return db;
}

/**
 * @brief Create Kwest database for first use
 * @param void
 * @return KW_SUCCESS : SUCCESS
 * @author SG
 */
int create_db(void)
{
	char query[QUERY_SIZE];
	int status;
	char *homedir, *username;

	strcpy(query,"create table if not exists FileDetails "
	"(fno integer primary key,fname text,abspath text);");
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	strcpy(query,"create table if not exists TagDetails "
	"(tno integer primary key,tagname text);");
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	strcpy(query,"create table if not exists FileAssociation "
	"(tno integer,fno integer);");
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	strcpy(query,"create table if not exists TagAssociation "
	"(t1 integer,t2 integer,associationid integer);");
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	strcpy(query,"create table if not exists MetaInfo "
	"(filetype text,tag text);");
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	strcpy(query,"create table if not exists Associations "
	"(associationid integer primary key,associationtype text);");
	status = sqlite3_exec(get_kwdb(),query,0,0,0);

	strcpy(query,"create table if not exists AssociationRules "
	"(type integer,conf real,tag1 text,tag2 text);");
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

	get_homedir(&homedir);
	username = strrchr(homedir, '/') + 1;
	add_tag(username, SYSTEM_TAG);
	add_association(username, TAG_ROOT, ASSOC_SUBGROUP);

	return status;
}

/**
 * @brief Close Kwest Database Connection
 * @param void
 * @return KW_SUCCESS : SUCCESS
 * @author SG
 */
int close_db(void)
{
	int status;

	status = sqlite3_close(get_kwdb());

	if (status != SQLITE_OK) {
		log_msg("%s", ERR_DB_CLOSE);
	} else {
		log_msg("%s", SUC_DB_CLOSE);
	}

	return status;
}

/**
 * @brief Begin transaction
 * @param void
 * @return KW_SUCCESS : SUCCESS
 * @author SG
 */
int begin_transaction(void)
{
	return (int)sqlite3_exec(get_kwdb(),"BEGIN",0,0,0);
}

/**
 * @brief Commit transaction
 * @param void
 * @return KW_SUCCESS : SUCCESS
 * @author SG
 */
int commit_transaction(void)
{
	return (int)sqlite3_exec(get_kwdb(),"COMMIT",0,0,0);
}
