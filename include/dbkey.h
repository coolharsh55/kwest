/**
 * @file dbkey.h
 * @brief handle requests for files and tags
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

#ifndef DBKEY_H_INCLUDED
#define DBKEY_H_INCLUDED


/*
 * Generate id for new file to be added in kwest
 */
int set_file_id(const char *abspath);

/*
 * Generate id for new tag to be created in kwest
 */
int set_tag_id(const char *tagname,int tagtype);

/*
 * Return id for file in kwest
 */
int get_file_id(const char *abspath);

/*
 * Return id for tag in kwest
 */
int get_tag_id(const char *tagname);

/*
 * Retrieve filename by its id
 */
const char *get_file_name(int fno);

/*
 * Retrieve tag name by its id
 */
const char *get_tag_name(int tno);

#endif
