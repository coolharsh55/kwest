/* dbfuse.c
 * functions connecting fuse and database modules
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

#include "dbfuse.h"

#include "flags.h"
#include "dbbasic.h"
#include "logging.h"

#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>


/* log_init
 * initialize logging
 */
bool _is_path_root(const char *path) 
{
	if(*(path+1) == '\0') {
		return true;
	}
	else {
		return false;
	}
}


/* get_entry_name
 * get last filename/tagname from path
 */
static const char *get_entry_name(const char *path)
{
	return (strrchr(path, '/') + 1);
}


/* check_association
 * check if association exists between consecutive tags in path
 */
static int check_association(const char *path)
{
	char *tmp_ptr,*tmp_path;
	char *tag1,*tag2;
	int assn;
	
	tmp_path = strdup(path);
	
	while( (tmp_ptr = strrchr(tmp_path,'/')) != NULL)
	{
		tag1 = tmp_ptr + 1;
		*tmp_ptr = '\0';
		
		if((tmp_ptr = strrchr(tmp_path,'/')) == NULL) {
			break;
		}
		tag2 = tmp_ptr + 1;
		
		assn = get_association(tag1,tag2);
		if(assn == KW_ERROR || assn == KW_FAIL) {
			return KW_FAIL;
		}
	}
	
	free((char*)tmp_path);
	return KW_SUCCESS;
}


/* check_path_validity
 * checks whether current path is valid in database
 */
int check_path_validity(const char *path)
{
	char *tmp_path,*tmp_ptr;
	
	if(*(path + 1) == '\0') {
		return KW_SUCCESS;
	}
	if(istag(get_entry_name(path)) == true) {
		if(check_association(path) == KW_SUCCESS) {
			return KW_SUCCESS;
		}
	} else if(isfile(get_entry_name(path)) == true) {
		tmp_path = strdup(path);
		tmp_ptr = strrchr(tmp_path,'/');
		*tmp_ptr='\0';
		if(check_association(tmp_path) == KW_SUCCESS) {
			free((char *)tmp_path);
			return KW_SUCCESS;
		}
		free((char *)tmp_path);
	} else {
		return -ENOENT;	
	}
	/*
	 * const char *tag1 = strchr(path);
	 * if(tag1 == path) { / * no of entries = 1 * /
		 * return KW_SUCCESS;
	 * }
	 * 
	 * const char *tag2 = strchr(tag1 - 1);
	 * 
	 */
	return KW_FAIL;
}


/* path_is_dir
 * checks whether given path has a directory entry
 */
bool path_is_dir(const char *path)
{
	if(istag(get_entry_name(path)) != true)
		return false;
	return true;
}


/* path_is_file
 * checks whether given path has a file entry
 */
bool path_is_file(const char *path)
{
	if(isfile(get_entry_name(path)) != true) {
		return false;
	}
	
	return true;
}


/* get_absolute_path
 * returns absolute path of said file
 */
const char *get_absolute_path(const char *path)
{
	return get_abspath_by_fname(get_entry_name(path));
}


/* readdir_dirs
 * get directory entries for said path
 */
char *readdir_dirs(const char *path, void **ptr)
{
	log_msg ("readdir_dirs: %s\n",path);
	if(*ptr == NULL) {
		if(*(path + 1) == '\0') {
			*ptr=get_tags_by_association(TAG_ROOT, ASSOC_SUBGROUP);
		} else {
			const char *t = strrchr(path,'/');
			*ptr = get_tags_by_association(t + 1, ASSOC_SUBGROUP); 
		}
		if(*ptr == NULL) {
			log_msg("pointer p is NULL\n");
			return NULL;
		}
	}
	
	return (char *)string_from_stmt(*ptr);
}


/* readdir_files
 * get file entries for said path
 */
char *readdir_files(const char *path, void **ptr)
{
	log_msg ("readdir_files: %s\n",path);
	if(*ptr == NULL) {
		if(*(path + 1) == '\0') {
			*ptr = get_fname_under_tag(TAG_ROOT);
		} else {
			const char *t = strrchr(path,'/');
			*ptr = get_fname_under_tag(t + 1); 
		}
		if(*ptr == NULL) {
			return NULL;
		}
	}
	
	return (char *)string_from_stmt(*ptr);
}


/* get_newfile_path
 * create a new file and return is absolute path
 */
const char *get_newfile_path(const char *path)
{
	return NULL;
}


/* rename_file
 * rename the said file from -> to
 */
int rename_this_file(const char *from, const char *to)
{
	return rename_file(from,to);
}


/* remove_file
 * remove the said file
 */
int remove_this_file(const char *path)
{
	
	char *filename = (char *)get_entry_name(path);
	char *tptr = filename -2;
	char *tagname = NULL;
	char *t2 = NULL;
	
	while(*tptr != '/') tptr--; /* seperat directory name */
	tptr++;
	
	tagname = malloc(filename - tptr);
	t2 = tagname;
	while(*tptr != '/') {
		*t2 = *tptr;
		t2++; tptr++;
	}
	*t2 = '\0';
	
	log_msg ("remove_this_file: %s",path);
	log_msg ("filename: %s",filename);
	log_msg("tagname: %s", tagname);
	
	if(untag_file(tagname, filename) == KW_SUCCESS) {
		log_msg("untag file successful");
		free(tagname);
		return KW_SUCCESS;
	}
	free(tagname);
	log_msg("untag file has failed");
	
	return KW_FAIL;
}


/* make_directory
 * create a new directory
 */
int make_directory(const char *path, mode_t mode)
{
	char *newtag = NULL;
	char *tptr = NULL;
	char *parenttag = NULL;
	char *t2 = NULL;
	
	log_msg ("make_directory: %s",path);
	newtag = (char *)get_entry_name(path);
	
	if(add_tag(newtag,USER_TAG) != KW_SUCCESS) {
		log_msg ("make_directory: failed to add tag %s",newtag);
		return KW_FAIL;
	}
	
	tptr = newtag -2;
	while(*tptr != '/') tptr--;
	tptr++;
	
	parenttag = malloc(newtag - tptr);
	t2 = parenttag;
	while(*tptr != '/') {
		*t2 = *tptr;
		t2++; tptr++;
	}
	*t2 = '\0';
	
	if(add_association(newtag, parenttag, ASSOC_SUBGROUP) != KW_SUCCESS) {
		log_msg ("make_directory: failed to add association ");
		return KW_FAIL;
	}
	
	log_msg ("make_directory: success");
	return KW_SUCCESS;
}


/* remove_directory
 * remove directory
 */
int remove_directory(const char *path)
{
	log_msg ("remove_directory: %s",path);
	
	if(remove_tag(get_entry_name(path)) == KW_SUCCESS) {
		return KW_SUCCESS;
	}
	
	return KW_FAIL;
}
