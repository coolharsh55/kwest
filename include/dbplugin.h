/**
 * @file dbplugin.h
 * @brief Wrapper functions for interaction with database
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

#ifndef DBPLUGIN_H_INCLUDED
#define DBPLUGIN_H_INCLUDED

/*
 * Add new mime type to Kwest
 */
void add_mime_type(char *mime);

/*
 * Add new metadata type to existing mime type
 */
void add_metadata_type(char *mime, char *metadata);

/*
 * Form association for metadata in file
 */
int associate_file_metadata(const char *mime,const char *tagname,
                            const char *fname);

#endif
