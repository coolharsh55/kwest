/* magicstrings.h
 * magic strings (!!!)
 */

/* LICENSE
 * Copyright 2013 Harshvardhan Pandit
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

#ifndef KWEST_MAGICSTRINGS_H
#define KWEST_MAGICSTRINGS_H




#define ERR_TAG_EXISTS "Tag Exists : "
#define ERR_TAG_NOT_FOUND "Tag Not Found : "
#define ERR_FILE_EXISTS "File Exists : "
#define ERR_FILE_NOT_FOUND "File Not Found : "
#define ERR_REL_NOT_DEF "Relation Not Defined : "
#define ERR_INF_EXISTS "Information Exists : "
#define ERR_ADDING_FILE "Error adding file : "
#define ERR_ADDING_META "Error adding Metadata for file : "
#define ERR_ADDING_TAG "Error adding tag : "
#define ERR_REMV_FILE "Error removing file : "
#define ERR_REMV_TAG "Error removing tag : "
#define ERR_PREP_QUERY "Error Preparing query"
#define ERR_RENAMING_FILE "Error renaming file : "

#define ERR_DB_CONN "Database Connection Failed"
#define ERR_DB_CLOSE "Error Closing Database"

#define CONFIG_LOCATION "/.config/kwest/"
#define DATABASE_NAME "kwest.db"

#define LOGFILE_STORAGE "logfile.log"


#define ASSOC_SYSTEM "system"
#define ASSOC_PROBAB "probably_related"
#define ASSOC_SUBGRP "subgroup"
#define ASSOC_RELATD "related"
#define ASSOC_NOTREL "not_related"

#define TAG_ROOT "Root"
#define TAG_FILES "Files"

#define TAG_AUDIO "Audio"
#define TAG_TITLE "Title"
#define TAG_ALBUM "Album"
#define TAG_ARTIST "Artist"
#define TAG_GENRE "Genre"

#define TAG_UNKNOWN "Unknown"

#endif
