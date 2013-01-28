/* dbkey.h
 * handle requests for files and tags
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

#ifndef DBKEY_H_INCLUDED
#define DBKEY_H_INCLUDED

/* set_file_id: Generate id for new file to be added in kwest
 * param: char *abspath - Absolute Path of File
 * return: int fno
 * author: @SG 
 */
int set_file_id(const char *abspath);

/* set_tag_id: Generate id for new tag to be created in kwest
 * param: char *tagname
 * return: int tno 
 * author: @SG 
 */
int set_tag_id(const char *tagname,int tagtype);

/* get_file_id: return id for file in kwest
 * param: char *abspath - Absolute Path of File
 * return: int fno 
 * author: @SG 
 */
int get_file_id(const char *abspath);

/* get_tag_id: return id for a tag in kwest
 * param: char *tagname
 * return: int tno 
 * author: @SG 
 */
int get_tag_id(const char *tagname);

/* get_file_name: Retrieve filename by its id
 * param: int fno - file number
 * return: char *filename
 * author: @SG 
 */
const char *get_file_name(int fno);

/* get_tag_name: Retrieve tag name by its id
 * param: int tno - tag number
 * return: char *tagname
 * author: @SG 
 */
const char *get_tag_name(int tno);

#endif
