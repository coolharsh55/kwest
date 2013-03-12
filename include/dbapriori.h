/**
 * @file dbapriori.h
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

#ifndef DBAPRIORI_H_INCLUDED
#define DBAPRIORI_H_INCLUDED

#include <sqlite3.h>

/* -------------------- Apriori Functions --------------------- */

/*
 * Count of all user created tags in kwest
 */
int count_user_tags(void);

/*
 * List all user tags in the system
 */
sqlite3_stmt *get_user_tagname(void);

/*
 * Get fno of user tagged files
 */
sqlite3_stmt *get_user_tagged_files(void);

/*
 * Get fno of user tagged files
 */
int add_rule(char *para1, char *para2);

/*
 * Get file suggestions using apriori association rules
 */
char *get_file_suggestions(char *tagname);

/*
 * Finalize sqlite statement
 */
void finalize(sqlite3_stmt * stmt);

#endif
