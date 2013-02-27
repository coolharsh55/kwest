/**
 * @file flags.h
 * @brief Flags used by KWEST programs
 * @author Harshvardhan Pandit
 * @author Sahil Gupta
 * @date December 2012
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

#ifndef KWEST_FLAGS_H
#define KWEST_FLAGS_H

#include <errno.h>
#include <stdbool.h>
#include "magicstrings.h"


/* RETURN VALUES FOR KWEST FUNCTIONS */
#define KW_SUCCESS 0 /* return SUCCESS as status */
#define KW_FAIL   -1 /* return FAILED as status */
#define KW_ERROR   1 /* return ERROR as status */


/* FLAGS RELATED TO FUSE, DIRECTORY VALUES */
#define KW_STDIR 0755 /* DIR entry in struct stat */
#define KW_STFIL 0444 /* FILE entry in struct stat */


/* FLAGS RELATED TO DATABASE OPERATIONS */
#define QUERY_SIZE 512 /* Size of array holding query */

#define USER_TAG   1 /* Tag Accessible to user */
#define SYSTEM_TAG 2 /* Tag created and used by system */

/* Starting id for tags and files */
#define NO_DB_ENTRY      0
#define FILE_START       0
#define SYSTEM_TAG_START 0
#define USER_TAG_START   100

/* Tag-Tag Association Types */
#define ASSOC_PROBABLY_RELATED 1
#define ASSOC_SUBGROUP         2
#define ASSOC_RELATED          3
#define ASSOC_NOT_RELATED      4

#endif
