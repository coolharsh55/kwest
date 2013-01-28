/* dbfuse.h
 * functions connecting fuse and database modules
 */

/* LICENSE
 * Copyright 2013 Harshvardhan pandit
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
 
 #ifndef KWEST_DBFUSE_H
 #define KWEST_DBFUSE_H

#include "flags.h"
#include <sys/stat.h>


/* _is_path_root
 * checks whether given path is ROOT
 * 
 * @param const char *path
 * @return true if _is_root else false
 * @author @HP
 */
bool _is_path_root(const char *path);


/* check_path_validity
 * checks whether current path is valid in database
 * 
 * @param const char *path
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author @HP
 */
int check_path_validity(const char *path);


/* path_is_dir
 * checks whether given path has a directory entry
 * 
 * @param const char *path
 * @return true if _is_dir else false
 * @author @HP
 */
bool path_is_dir(const char *path);


/* path_is_file
 * checks whether given path has a file entry
 * 
 * @param const char *path
 * @return true if _is_file else false
 * @author @HP
 */
bool path_is_file(const char *path);


/* get_absolute_path
 * returns absolute path of said file
 * 
 * @param const char *path
 * @return char * as file absolute path
 * @author @HP
 */
const char *get_absolute_path(const char *path);


/* readdir_dirs
 * get directory entries for said path
 * 
 * @param const char *path
 * @return char * as directory entry
 * @author @HP
 */
char *readdir_dirs(const char *path, void **ptr);


/* readdir_files
 * get file entries for said path
 * 
 * @param const char *path
 * @return char * as file entry
 * @author @HP
 */
char *readdir_files(const char *path, void **ptr);


/* get_newfile_path
 * create a new file and return is absolute path
 * 
 * @param const char *path
 * @return const char * as absolute path
 * @author @HP
 */
const char *get_newfile_path(const char *path);


/* rename_file
 * rename the said file from -> to
 * 
 * @param const char *from : SOURCE
 * @param const char *to : DESTINATION
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author @HP
 */
int rename_this_file(const char *from, const char *to);


/* remove_file
 * remove the said file
 * 
 * @param const char *path
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author @HP
 */
int remove_this_file(const char *path);


/* make_directory
 * create a new directory
 * 
 * @param const char *path
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author @HP
 */
int make_directory(const char *path, mode_t mode);


/* remove_directory
 * remove directory
 * 
 * @param const char *path
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author @HP
 */
int remove_directory(const char *path);

 #endif
