/* import.c
 * import files into kwest
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
 * int add_tag(char *tagname,int tagtype)
 * int add_file(char *abspath)
 * int tag_file(char *t,char *f)
 * int add_association(char *t1,char *t2,int associationid)
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h> /* stat structure */
#include <dirent.h>   /* DIR */
#include <sqlite3.h>  /* SQLITE Functions */

#include "dbbasic.h" /* Kwest Datadase Functions */
#include "extract_audio_taglib.h" /* Extract Audio Metadata */ 

#include "import.h"
#include "flags.h"
#include "magicstrings.h"
#include "logging.h"

/* import_semantics
 * import files and directories into kwest
 */
static int import_semantics(const char *path,const char *dirname)
{
	char full_name[_POSIX_PATH_MAX + 1];
	DIR *directory = opendir(path);
	struct dirent *entry = NULL; 
	struct stat fstat;
	size_t path_len = strlen(path);
	size_t dir_len;

	log_msg("import semantics: %s into %s", path, dirname);
	 
	if (directory == NULL) {
		log_msg("ERROR: Couldn't open the directory");
		perror ("Couldn't open the directory");
		return KW_FAIL;
	}

	while ((entry = readdir(directory))) {
		dir_len = strlen(entry->d_name);
		
		/* Calculate full name, check we are in file length limts */
		if ((path_len + dir_len + 1) > _POSIX_PATH_MAX){
			continue;
		}
		strcpy(full_name, path);
		if (full_name[path_len - 1] != '/'){
			strcat(full_name, "/");
		}
		strcat(full_name, entry->d_name);

		/* Ignore files starting with . */
		if((strchr(entry->d_name,'.')-entry->d_name) == 0){
			continue;
		}

		/* Ignore files ending with ~ */
		if((strrchr(entry->d_name,'~')-entry->d_name) == dir_len-1){
			continue;
		}

		/* Get File Infromation : Returns 0 if successful */
		if (stat(full_name, &fstat) < 0){
			continue;
		}

		if (S_ISDIR(fstat.st_mode)) { /* Directory */

			if(add_tag(entry->d_name,USER_TAG) == KW_SUCCESS){
				printf("Created Tag : %s",entry->d_name);
			}

			/* Access Sub-Directories */
			import_semantics(full_name,entry->d_name);

			/* Tag-Tag Relation */
			add_association(entry->d_name,dirname,
			                ASSOC_SUBGROUP);

		} else if(S_ISREG(fstat.st_mode)) { /* Regular File */

			if(add_file(full_name) == KW_SUCCESS){
				printf("Added File  : %s\n",entry->d_name);

				/* Tag-File Relation */
				tag_file(dirname,entry->d_name);
			}
		}
	}

	closedir (directory);
	return KW_SUCCESS;
}

/* import
 * Import Directory-File structure from File System to Kwest
 */
int import(const char *path)
{
	/* Extract Directory name from path */
	const char *dirname = strrchr(path,'/') + 1; 
	
	log_msg("import: %s", path);
	
	/* Create Tag for directory to be imported */
	if(add_tag(dirname, USER_TAG) == KW_SUCCESS){
		printf("Creating Tag : %s\n",dirname);
		add_association(dirname, TAG_FILES, ASSOC_SUBGROUP);
	}

	if (import_semantics(path, dirname) == KW_SUCCESS) {
		return KW_SUCCESS;
	}

	printf("Operation Failed");
	return KW_FAIL;
}
