/**
 * @file apriori.h
 * @brief apriori algorithm
 * @author Sahil Gupta
 * @date Februrary 2012
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

#ifndef APRIORI_H_INCLUDED
#define APRIORI_H_INCLUDED

#define MINSUP 0.4
#define MINCONF 0.5
#define RULE_LEVEL 0.5

#define MAX_ITEMSET_LENGTH 1024
#define MAX_ITEM_LENGTH 5
#define MIN_ITEM_LENGTH 3

#define CHAR_ITEMSET_SEP '|'
#define CHAR_ITEM_SEP ','

#define STR_ITEMSET_SEP "|"
#define STR_ITEM_SEP ","

typedef struct Candidate{
	char members[MAX_ITEMSET_LENGTH];
	int supportcnt[(int)(MAX_ITEMSET_LENGTH / (MIN_ITEM_LENGTH + 1))];
	struct Candidate *nextc;
}C;

typedef struct Itemset{
	int number;
	int count;
	C *candidate;
	struct Itemset *nexti;
}I;
I *headi;

/*
 * Database Functions
 *
 * FILE SUGGESTIONS
 * generate_candidates :
 ** 	sqlite3 stmt *get_user_tagged_files(void)
 *
 * calculate_frequent_itemsets :
 ** 	sqlite3 stmt *get_user_tagname(void)
 ** 	sqlite3_stmt *get_fid_under_tag(const char *tagname)
 *
 * apriori :
 ** 	int count_user_tags(void)
 *
 *
 * GENERAL
 * complete_rule :
 * 	add_rule(char *para1, char *para2)
 *
 * Others :
 * 	char *string_from_stmt(sqlite3_stmt * stmt)
 * 	void finalize(sqlite3_stmt * stmt)
 */

/*
 * Returns new token separated by separator
 */
void get_token(char **token, char *source_str ,int tokenno, char separator);

/*
 * Returns individual items from a token
 */
void get_item(char **item, char *token,int *item_iter);

/*
 * Total number of items in token
 */
int get_no_of_items(char *token);

/*
 * Check if item is present in itemset
 */
int check_item(char *itemset, char *item, int count, char sep);

/*
 * Apriori Algorithm
 */
void apriori(void);

#endif
