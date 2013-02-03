/**
 * @file dbbasic.h
 * @brief basic interation with database
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

#ifndef DBBASIC_H_INCLUDED
#define DBBASIC_H_INCLUDED

#include <sqlite3.h>
#include "flags.h"


/* ---------------- ADD/REMOVE -------------------- */

/* 
 * Create a new tag in kwest
 */
int add_tag(const char *tagname,int tagtype);

/*
 * Remove an existing tag from kwest 
 */
int remove_tag(const char *tagname);

/*
 * Add file to kwest
 */
int add_file(const char *abspath);

/*
 * Remove file form kwest 
 */
int remove_file(const char *path);

/*
 * Add new category to identify metadata 
 */
int add_meta_info(const char *filetype,const char *tag);

/* --------------- Tag-File Relation ------------- */

/*
 * Associate a tag with a file 
 */
int tag_file(const char *t,const char *f);

/*
 * Remove the existing association between the tag and file
 */
int untag_file(const char *t,const char *f);

/*
 * Return list of files associated to given tag 
 */
sqlite3_stmt *get_fname_under_tag(const char *t);

/*
 * Return list of tags associated with a given file
 */
sqlite3_stmt *get_tags_for_file(const char *f);


/*----------------- Tag-Tag Relation ------------------*/

/*
 * Associate a tag with another tag 
 */
int add_association(const char *t1,const char *t2,int associationid);

/*
 * Remove the existing association between tags 
 */
int remove_association(const char *t1,const char *t2,int associationid);

/*
 * Return type of association between tags
 */
int get_association(const char *t1,const char *t2);

/*
 * Get tag having particular association with other tag 
 */
sqlite3_stmt* get_tags_by_association(const char *t,int associationid);

/* ----------------- Associations --------------------- */

/*
 * Create a new association type 
 */
int add_association_type(const char *associationtype);

/*
 * Check if relation exists
 */
int is_association_type(int associationid);

/* --------------------- Others --------------------- */

/*
 * List all tags in the system 
 */
sqlite3_stmt *list_user_tags(void);

/*
 * Returns data for multiple rows in query
 */
const char* string_from_stmt(sqlite3_stmt *stmt);

/* -------------------- Fuse Functions --------------------- */

/*
 * Check if given tag is present in system
 */
bool istag(const char *t);

/*
 * Check if given file is present in system
 */
bool isfile(const char *f);

/*
 * Return absolute path of file
 */
char *get_abspath_by_fname(const char *fname);

/*
 * Rename file existing in kwest
 */
int rename_file(const char *from, const char *to);

#endif
