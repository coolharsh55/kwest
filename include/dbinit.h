/**
 * @file dbinit.h
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

#ifndef DBINIT_H_INCLUDED
#define DBINIT_H_INCLUDED

#include <sqlite3.h>


/*
 * Get users home directory absolute path
 */
void get_homedir(char **homedir);


/*
 * Initialize Return sqlite pointer object
 */
sqlite3 *get_kwdb(void);

/*
 * Create Kwest database for first use
 */
int create_db(void);

/*
 * Close Kwest Database Connection
 */
int close_db(void);

/*
 * Begin transaction
 */
int begin_transaction(void);

/*
 * Commit transaction
 */
int commit_transaction(void);

#endif
