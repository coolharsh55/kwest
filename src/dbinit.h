/* dbinit.c
 * create and initialize the database
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


/* ------------ Database Connections -------------- */

/* get_kwdb: Initialize Return sqlite* Object
 * return: sqlite3 pointer
 * author: @SG
 */
sqlite3 *get_kwdb(void);

/* create_db: Create Kwest database for first use
 * return: 0 on SUCCESS
 * author: @SG
 */
int create_db(void);

/* close_db: Close Kwest Database Connection
 * return: 0 on SUCCESS
 * author: @SG
 */
int close_db(void);

/* begin_transaction: 
 * return: 0 on SUCCESS
 * author: @SG
 */
int begin_transaction(void);

/* commit_transaction: 
 * return: 0 on SUCCESS
 * author: @SG
 */
int commit_transaction(void);

#endif
