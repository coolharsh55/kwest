/* export.h
 * export kwest path & files to external location
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


/* Database Functions Required:
 * int close_db(void);
 * sqlite3_stmt *get_association(char *tag,int relationid);
 * sqlite3_stmt *get_file_abspath(char *tag);
 * const char* get_data(sqlite3_stmt *stmt);
 */


/* export: Exports a tag in kwest as Directory-file structure on File System
 * param: char *tag - Tag name in Kwest
 * param: char *path - Absolute Path of Location where tag is to be exported
 * return: KW_SUCCESS on SUCCESS
 * author: @SG
 */
int export(const char *tag,const char *path);
