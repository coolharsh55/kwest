/**
 * @file dbapriori.c
 * @brief Wrapper function for interaction with database
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
#include <pwd.h>
#include <unistd.h>

#include "dbapriori.h"
#include "dbbasic.h"
#include "dbinit.h"
#include "dbkey.h"
#include "apriori.h"
#include "logging.h"
#include "flags.h"
#include "magicstrings.h"

/* --------------------------- LOCAL FUNCTIONS ------------------------------ */

static int get_data_string_under_tag(char *tagname, char **data_str,
                                   sqlite3_stmt *(*get_id)(const char *tagname))
{
	sqlite3_stmt *stmt;
	*data_str = (char *) malloc(MAX_ITEMSET_LENGTH * sizeof(char));
	const char *id;
	int cnt;

	cnt = 0;
	strcpy(*data_str, "");

	/* Get all data under the tag tagname */
	stmt = (*get_id)(tagname);
	while((id = string_from_stmt(stmt)) != NULL) {
		if(strlen(*data_str) < (MAX_ITEMSET_LENGTH -
		                        (MAX_ITEM_LENGTH + 2))) {
			strcat(*data_str, id);
			strcat(*data_str, ",");
			cnt++;
		} else {
			/** @TODO : reallocate memory for data_str */
			log_msg("Insufficient memory to retrive data in tag");
			finalize(stmt);
			break;
		}
	}
	if(strcmp(*data_str, "") != 0) {
		*(*data_str + strlen(*data_str) - 1) = '\0';
	}

	return cnt;
}


static int check_itemset(char *itemset, char *main_itemset, int maincnt)
{
	char *token;
	int itemsetcnt;
	int i;

	itemsetcnt = get_no_of_items(itemset);
	token = (char *)malloc(strlen(itemset) * sizeof(char));

	for(i = 0; i < itemsetcnt; i++) {
		get_token(&token, itemset, i, CHAR_ITEM_SEP);
		if(check_item(main_itemset, token, maincnt, CHAR_ITEM_SEP) == 0)
		{
			free((char *) token);
			return 0; /* Itemset not present in mainitemset */
		}
	}

	free((char *) token);
	return 1;
}

static void correct_items(char ***itemset, int *cnt, char *reference, int refcnt, const char *(*get_name)(int id))
{
	int i;
	char *tmpset = (char *)malloc(MAX_ITEMSET_LENGTH * sizeof(char));
	const char *tmpname;
	int tmpcnt;
	char *token;

	token = (char *)malloc(strlen(**itemset) * sizeof(char));
	strcpy(tmpset, "");
	tmpcnt = 0;

	for(i = 0; i < *cnt; i++) {
		get_token(&token, **itemset, i, CHAR_ITEM_SEP);
		if(check_item(reference, token, refcnt, CHAR_ITEM_SEP) == 0) {
			tmpname = (*get_name)(atoi(token));
			if(strlen(tmpset) < (MAX_ITEMSET_LENGTH -
			                     strlen(tmpname) - 1)) {
				strcat(tmpset, tmpname);
				strcat(tmpset, STR_ITEM_SEP);
				tmpcnt++;
			} else {
				log_msg("Insufficient memory to compile "
				        "suggested file name");
				break;
			}
		}
	}

	if(strcmp(tmpset, "") != 0) {
		tmpset[strlen(tmpset) - 1] = '\0';
	}

	strcpy(**itemset, tmpset);
	*cnt = tmpcnt;
	free((char *) token);
	free((char *) tmpset);
}

/**
 * @brief Get suggestions using apriori association rules
 * @param tagname
 * @param suggest File Suggestions String Separated by CHAR_ITEM_SEP
 * @return void
 * @author SG
 */
static void get_suggestions(char *tagname, char **suggest, int type,
    sqlite3_stmt *(get_id)(const char *tagname),const char *(*get_name)(int id))
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	char *data_str;
	int datacnt;
	char *lhsrule,*rhsrule;
	int suggestcnt;
	char *token;
	int i;

	strcpy(*suggest, "");
	suggestcnt = 0;

	datacnt = get_data_string_under_tag(tagname, &data_str, get_id);
	//log_msg("Tag : %s #%d Files : %s", tagname, datacnt, data_str);

	/* analyze all association rules */
	sprintf(query,"select tag1,tag2 from AssociationRules where type = %d",
	        type);
	sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	do {
		/* Get individual rules */
		if(SQLITE_ROW != sqlite3_step(stmt)) {
			break;
		}

		/* Check if items in LHS of rule are present */
		lhsrule = (char*)sqlite3_column_text(stmt,0);
		if( check_itemset(lhsrule, data_str, datacnt) == 0) {
			continue; /* If not goto next rule */
		}

		/* Check if items in RHS of rule are present */
		rhsrule = (char*)sqlite3_column_text(stmt,1);
		if( check_itemset(rhsrule, data_str, datacnt) == 1) {
			continue; /* If yes, suggested data already present */
		}

		/* Rule Applies, append to suggestions */
		//log_msg("%s->%s",lhsrule,rhsrule);

		token = (char *)malloc(strlen(rhsrule) * sizeof(char));
		/* Remove Duplication */
		for(i = 0; i < get_no_of_items(rhsrule); i++) {
			get_token(&token, rhsrule, i, CHAR_ITEM_SEP);
			if( check_item(*suggest, token, suggestcnt,
			               CHAR_ITEM_SEP) == 0) {
				if(strlen(*suggest) < (MAX_ITEMSET_LENGTH -
				                      strlen(token) - 1) ) {
					strcat(*suggest,token);
					strcat(*suggest,STR_ITEM_SEP);
					suggestcnt++;
				} else {
					log_msg("Insufficient memory to compile"
					        " suggested data");
					break;
				}
			}
		}
		free((char *)token);
	} while(1);
	sqlite3_finalize(stmt);

	if(strcmp(*suggest, "") != 0) {
		*(*suggest + strlen(*suggest) - 1) = '\0';
	}
	correct_items(&suggest, &suggestcnt, data_str, datacnt, get_name);
	free((char *) data_str);
}

/* ---------------------------- GENERAL ------------------------------------- */

/**
 * @brief Count of all user created tags in kwest
 * @param void
 * @return Count
 * @author SG
 */
int count_user_tags(void)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;
	int tmp; /* To Hold result of Query */

	sprintf(query,"select count(*) from TagDetails where tno>=%d;",
	             USER_MADE_TAG);
	sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

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
 * @brief List all user tags in the system
 * @param void
 * @return sqlite3_stmt pointer: SUCCESS, NULL : FAIL
 * @author SG
 */
sqlite3_stmt *get_user_tagname(void)
{
	sqlite3_stmt * stmt;
	char query[QUERY_SIZE];
	int status;

	/* Query to get user tags from Database */
	sprintf(query,"select tagname from TagDetails where tno>=%d;",
	              USER_MADE_TAG);
	status = sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	if(status != SQLITE_OK){ /* Error Preparing query */
		log_msg("get_all_tags : %s",ERR_PREP_QUERY);
		return NULL;
	}

	return stmt;
}

/**
 * @brief Add association rule to database
 * @param void
 * @return sqlite3 stmt pointer
 * @author SG
 *
 * @todo Remove unwanted association rules from db after adding new rules
 */
int add_rule(int type, char *para1, char *para2)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;

	/* Query : check if entry exists in AssociationRule Table */
	sprintf(query,"select count(*) from AssociationRules where "
	              "tag1 = '%s' and tag2 = '%s' and type = %d;",
	              para1, para2, type);
	status = sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	status = sqlite3_step(stmt);
	if(status == SQLITE_ROW) {
		if(atoi((const char*)sqlite3_column_text(stmt,0))!= 0) {
			sqlite3_finalize(stmt);
			return KW_ERROR; /* Rule Exists */
		}
	}
	sqlite3_finalize(stmt);
	stmt = NULL;

	/* Query : Insert in AssociationRule Table */
	strcpy(query,"insert into AssociationRules values"
	             "(:type,:para1,:para2);");
	sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	sqlite3_bind_int(stmt,1,type);
	sqlite3_bind_text(stmt,2,para1,-1,SQLITE_STATIC);
	sqlite3_bind_text(stmt,3,para2,-1,SQLITE_STATIC);

	status = sqlite3_step(stmt);

	if(status == SQLITE_DONE){
		sqlite3_finalize(stmt);
		return KW_SUCCESS;
	}

	log_msg("add_rule_error : %s -> %s",para1,para2);
	sqlite3_finalize(stmt);
	return KW_FAIL;
}

/* ---------------------------- FILE SUGGESTIONS----------------------------- */

/**
 * @brief Get fno of user tagged files
 * @param void
 * @return sqlite3 stmt pointer
 * @author SG
 */
sqlite3_stmt *get_user_tagged_files(void)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;

	sprintf(query,"select distinct fno from FileAssociation where tno >= %d"
	             " order by fno;", USER_MADE_TAG);
	status = sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	if(status != SQLITE_OK) { /* Error Preparing query */
		log_msg("get_user_tagged_files : %s",ERR_PREP_QUERY);
		return NULL;
	}

	return stmt;
}

char *get_file_suggestions(char *tagname)
{
	char *suggest = (char *) malloc(MAX_ITEMSET_LENGTH * sizeof(char));

	get_suggestions(tagname, &suggest, FILES, get_fid_under_tag, get_file_name);
	/* log_msg("suggest : %s",suggest); */

	if(strcmp(suggest,"") == 0) {
		free((char *) suggest);
		return NULL;
	}

	return suggest;
}

/* ---------------------------- TAG SUGGESTIONS ----------------------------- */

/**
 * @brief Get tno of user tagged tags
 * @param void
 * @return sqlite3 stmt pointer
 * @author SG
 */
sqlite3_stmt *get_user_tagged_tags(void)
{
	sqlite3_stmt *stmt;
	char query[QUERY_SIZE];
	int status;
	int tid;

	/* initial working directory */
	struct passwd *pw;
	char *homedir;

	pw = getpwuid(getuid());
	homedir = strrchr(pw->pw_dir,'/')+1;

	tid = get_tag_id(homedir);

	sprintf(query,"select distinct t1 from TagAssociation where t2 in "
	        "(select distinct t1 from TagAssociation where t2 = %d);", tid);
	status = sqlite3_prepare_v2(get_kwdb(),query,-1,&stmt,0);

	if(status != SQLITE_OK) { /* Error Preparing query */
		log_msg("get_user_tagged_tags : %s",ERR_PREP_QUERY);
		return NULL;
	}

	return stmt;
}

/**
 * @brief Get tag suggestions using apriori association rules
 * @param tagname
 * @return tag Suggestions String Separated by CHAR_ITEM_SEP
 * @author SG
 */
char *get_tag_suggestions(char *tagname)
{
	char *suggest = (char *) malloc(MAX_ITEMSET_LENGTH * sizeof(char));

	get_suggestions(tagname, &suggest, TAGS, get_tid_under_tag, get_tag_name);

	if(strcmp(suggest,"") == 0) {
		free((char *) suggest);
		return NULL;
	}

	return suggest;
}
/* ----------------------------- OTHERS ------------------------------------- */

/**
 * @brief Finalize sqlite statement
 * @param stmt sqlite3 statement pointer
 * @return void
 * @author SG
 */
void finalize(sqlite3_stmt * stmt)
{
	sqlite3_finalize(stmt);
}
