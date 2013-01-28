/* dbbasic.c
 * database interaction and functions
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

#ifndef DBFUNC_H_INCLUDED
#define DBFUNC_H_INCLUDED

#include <sqlite3.h>
#include "flags.h"

/* ---------------- ADD/REMOVE -------------------- */

/* add_tag
 * Create a new tag in kwest
 * 
 * param: char *tagname
 * return: 0 on SUCCESS
 * author: @SG
 */
int add_tag(const char *tagname,int tagtype);

/* remove_tag: Remove an existing tag from kwest 
 * param: char *tagname
 * return: 0 on SUCCESS
 * author: @SG
 */
int remove_tag(const char *tagname);

/* add_file: Add file to kwest
 * param: char *abspath
 * return: 0 on SUCCESS
 * author: @SG 
 */
int add_file(const char *abspath);

/* remove_file: Remove file form kwest 
 * param: char *path - Path in Kwest
 * return: 0 on SUCCESS
 * author: @SG 
 */
int remove_file(const char *path);

/* add_meta_info: Add new category to identify metadata 
 * param: filetype - Type of File
 * param: tag - Category of Metadata
 * return: 0 on SUCCESS
 * author: @SG  
 */
int add_meta_info(const char *filetype,const char *tag);

/* --------------- Tag-File Relation ------------- */

/* tag_file: Associate a tag with a file 
 * param: char *t - tagname
 * param: char *f - filename
 * return: 0 on SUCCESS
 * author: @SG
 */
int tag_file(const char *t,const char *f);

/* untag_file: Remove the existing association between the tag and file
 * param: char *t - tagname
 * param: char *f - filename
 * return: 0 on SUCCESS
 * author: @SG
 */
int untag_file(const char *t,const char *f);

/* get_fname_under_tag: Return list of files associated to given tag 
 * param: char *t - tagname
 * return: sqlite3_stmt *
 * author: @SG
 */
sqlite3_stmt *get_fname_under_tag(const char *t);

/* get_tags_for_file: Return list of tags associated with a given file
 * param: char *f - filename
 * return: sqlite3_stmt *
 * author: @SG
 */
sqlite3_stmt *get_tags_for_file(const char *f);


/*----------------- Tag-Tag Relation ------------------*/

/* add_association: Associate a tag with another tag 
 * param: char *t1,*t2 - tagname of both tags to be associated
 * param: int associationid - relation between tags to be formed
 * return: 0 on SUCCESS
 * author: @SG
 */
int add_association(const char *t1,const char *t2,int associationid);

/* remove_association: Remove the existing association between tags 
 * param: char *t1,*t2 - tagnames whose association is to be removed
 * param: int associationid - relation between tags to be removed
 * return: 0 on SUCCESS
 * author: @SG
 */
int remove_association(const char *t1,const char *t2,int associationid);

/* get_association: Return type of association between tags
 * param: char *t1,*t2 - tagname of both tags in association
 * return: int associationid
 * author: @SG
 */
int get_association(const char *t1,const char *t2);

/* get_tags_by_association: Get tag having particular association with other tag 
 * param: char *t - tagname
 * param: int associationid - relation between tags
 * return: sqlite3_stmt *
 * author: @SG 
 */
sqlite3_stmt* get_tags_by_association(const char *t,int associationid);

/* ----------------- Associations --------------------- */

/* add_association_type: Create a new association type 
 * param: char *associationtype
 * return: int associationid
 * author: @SG 
 */
int add_association_type(const char *associationtype);

/* is_association_type: Check if relation exists
 * param: int associationid
 * return: 1 if relation present
 * author: @SG
 */
int is_association_type(int associationid);

/* --------------------- Others --------------------- */

/* list_user_tags: List all tags in the system 
 * param: void
 * return: sqlite3_stmt *
 * author: @SG
 */
sqlite3_stmt *list_user_tags(void);

/* string_from_stmt: Returns data for multiple rows is query
 * param: sqlite3_stmt *stmt - statement holding query
 * return: const char* data returned by query
 * author: @SG 
 */
const char* string_from_stmt(sqlite3_stmt *stmt);

/* -------------------- Fuse Functions --------------------- */

/* istag: Check if given tag is present in system
 * param: char *t - tagname
 * return: 1 if tag present
 * author: @SG 
 */
bool istag(const char *t);

/* isfile: Check if given file is present in system
 * param: char *f - filename
 * return: 1 if file present
 * author: @SG 
 */
bool isfile(const char *f);

/* get_abspath_by_fname: return absolute path of file
 * param: char *path - kwest path
 * author: @SG 
 * return: char * absolute path
 */
char *get_abspath_by_fname(const char *path);

/* rename_file: rename file existing in kwest
 * param: char *from - existing name of file
 * param: char *to - new name of file
 * return: KW_SUCCESS, KW_ERROR, KW_FAIL
 * author: @SG 
 */
int rename_file(const char *from, const char *to);

#endif
