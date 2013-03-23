/**
 * @file dbfuse.c
 * @brief functions connecting fuse and database modules
 * @author Harshvardhan Pandit
 * @date December 2012
 */

/* LICENSE
 * Copyright 2013 Harshvardhan Pandit
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

#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "dbfuse.h"
#include "dbbasic.h"
#include "logging.h"
#include "flags.h"


/**
 * @brief checks whether given path is ROOT
 * @param path
 * @return true if _is_root else false
 * @author HP
 */
bool _is_path_root(const char *path) 
{
	if (*(path+1) == '\0') {
		return true;
	}
	else {
		return false;
	}
}

/**
 * @brief get last filename/tagname from path
 * @param path
 * @return entry_name
 * @author HP
 */
static const char *get_entry_name(const char *path)
{
	return (strrchr(path, '/') + 1 );
}

/**
 * @brief Check if association exists between consecutive tags in path
 * @param path
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL
 * @author SG
 */
static int check_association(const char *path)
{
	char *tmp_ptr,*tmp_path;
	char *tag1,*tag2;
	int assn;
	
	tmp_path = strdup(path);
	
	while ( (tmp_ptr = strrchr(tmp_path,'/')) != NULL)
	{
		tag1 = tmp_ptr + 1;
		*tmp_ptr = '\0';
		
		if ((tmp_ptr = strrchr(tmp_path,'/')) == NULL) {
			break;
		}
		tag2 = tmp_ptr + 1;
		
		assn = get_association(tag1,tag2);
		if (assn == KW_ERROR || assn == KW_FAIL) {
			return KW_FAIL;
		}
	}
	
	free((char*)tmp_path);
	return KW_SUCCESS;
}

/**
 * @brief checks whether current path is valid in database
 * @param path
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author HP SG
 */
int check_path_validity(const char *path)
{
	char *tmp_path,*tmp_ptr;
	/** @todo check path validity does not work correctly if the said
	 * filename already exists. It then assumes that the path is valid.
	 * Even if the file exists, it's associations with tags in the path
	 * should be checked and only then the path is validated
	 */
	if (*(path + 1) == '\0') {
		log_msg("is_root");
		return KW_SUCCESS;
	}
	/** @bug tagname could be NULL
	 * entry returned from here causes segmentation fault
	 */
	/*
	tmp_ptr = (char *)get_entry_name(path);
	if (tmp_ptr != NULL) {
		log_msg("tmp_ptr not NULL");
	} else {
		log_msg("tmp_ptr is NULL");
	}
	*/
	
	if (istag(get_entry_name(path)) == true) {
		if (check_association(path) == KW_SUCCESS) {
			log_msg("istag");
			return KW_SUCCESS;
		}
	} else if (isfile(get_entry_name(path)) == true) {
		tmp_path = strdup(path);
		tmp_ptr = strrchr(tmp_path,'/');
		*tmp_ptr='\0';
		if (check_association(tmp_path) == KW_SUCCESS) {
			if (is_file_tagged_as(tmp_ptr+1, 
			    strrchr(tmp_path,'/')+1) == false) {
				free((char *)tmp_path);
				return -ENOENT;
			}
			free((char *)tmp_path);
			return KW_SUCCESS;
		}
		free((char *)tmp_path);
	} else {
		return -ENOENT;	
	}
	/*
	 * const char *tag1 = strchr(path);
	 * if (tag1 == path) { / * no of entries = 1 * /
		 * return KW_SUCCESS;
	 * }
	 * 
	 * const char *tag2 = strchr(tag1 - 1);
	 * 
	 */
	return KW_FAIL;
}

/**
 * @brief checks whether current tags in path is valid in database
 * @param path
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author HP SG
 */
int check_path_tags_validity(const char *_path)
{
	char *path,*tmp_ptr;
	/** @todo check path validity does not work correctly if the said
	 * filename already exists. It then assumes that the path is valid.
	 * Even if the file exists, it's associations with tags in the path
	 * should be checked and only then the path is validated
	 */
	path = strdup(_path);
	log_msg("%s",path);
	if (*(path + 1) == '\0') {
		log_msg("is_root");
		free(path);
		return KW_SUCCESS;
	}
	/** @bug tagname could be NULL
	 * entry returned from here causes segmentation fault
	 */
	
	tmp_ptr = strrchr(path, '/'); *tmp_ptr = '\0';
	
	if (tmp_ptr != NULL) {
		log_msg("tmp_ptr not NULL");
	} else {
		log_msg("tmp_ptr is NULL");
	}
	
	
	if (istag(get_entry_name(path)) == true) {
		log_msg("istag PASS");
		if (check_association(path) == KW_SUCCESS) {
			log_msg("istag");
			free(path);
			return KW_SUCCESS;
		}
	/*
	} else if (isfile(get_entry_name(path)) == true) {
		tmp_path = strdup(path);
		tmp_ptr = strrchr(tmp_path,'/');
		*tmp_ptr='\0';
		if (check_association(tmp_path) == KW_SUCCESS) {
			if (is_file_tagged_as(tmp_ptr+1, 
			    strrchr(tmp_path,'/')+1) == false) {
				free((char *)tmp_path);
				return -ENOENT;
			}
			free((char *)tmp_path);
			return KW_SUCCESS;
		}
		free((char *)tmp_path);
		*/
	} else {
		free(path);
		return -ENOENT;	
	}
	/*
	 * const char *tag1 = strchr(path);
	 * if (tag1 == path) { / * no of entries = 1 * /
		 * return KW_SUCCESS;
	 * }
	 * 
	 * const char *tag2 = strchr(tag1 - 1);
	 * 
	 */
	free(path);
	return KW_FAIL;
}


/**
 * @brief checks whether given path has a directory entry
 * @param path
 * @return true if _is_dir else false
 * @author HP
 */
bool path_is_dir(const char *path)
{
	if (istag(get_entry_name(path)) != true)
		return false;
	return true;
}

/**
 * @brief checks whether given path has a file entry
 * @param path
 * @return true if _is_file else false
 * @author HP
 */
bool path_is_file(const char *path)
{
	if (isfile(get_entry_name(path)) != true) {
		return false;
	}
	
	return true;
}

/**
 * @brief returns absolute path of said file
 * @param path
 * @return const char * as file absolute path
 * @author HP
 */
const char *get_absolute_path(const char *path)
{
	return get_abspath_by_fname(get_entry_name(path));
}

/**
 * @brief get directory entries for said path
 * @param path
 * @param ptr
 * @return char * as directory entry
 * @author HP
 */
char *readdir_dirs(const char *path, void **ptr)
{
	/*log_msg ("readdir_dirs: %s",path);*/
	if (*ptr == NULL) {
		if (*(path + 1) == '\0') {
			*ptr=get_tags_by_association(TAG_ROOT, ASSOC_SUBGROUP);
		} else {
			const char *t = strrchr(path,'/');
			*ptr = get_tags_by_association(t + 1, ASSOC_SUBGROUP); 
		}
		if (*ptr == NULL) {
			return NULL;
		}
	}
	
	return (char *)string_from_stmt(*ptr);
}

/**
 * @brief get file entries for said path
 * @param path
 * @param ptr
 * @return char * as file entry
 * @author HP
 */
char *readdir_files(const char *path, void **ptr)
{
	/*log_msg ("readdir_files: %s",path);*/	
	if (*ptr == NULL) {
		if (*(path + 1) == '\0') {
			*ptr = get_fname_under_tag(TAG_ROOT);
		} else {
			const char *t = strrchr(path,'/');
			*ptr = get_fname_under_tag(t + 1); 
		}
		if (*ptr == NULL) {
			return NULL;
		}
	}
	
	return (char *)string_from_stmt(*ptr);
}

/**
 * @brief create a new file and return is absolute path
 * @param path
 * @return const char * as absolute path
 * @author HP
 */
const char *get_newfile_path(const char *path)
{
	/*
	 * @HP
	 * this function should get path to some real location
	 * so that the new file can be created there
	 * parse path - there will be the format
	 * /Files/Music/tags.../filename.ext
	 * so try to get directory /Music/tags... or something
	 * then return that...
	 * 
	 * char *path = some_new_path
	 * 
	 * return (const char *)path;
	 */
	(void)path;
	return NULL;
}

/**
 * @brief rename the said file from -> to
 * @param from - source file name
 * @param to - destination file name
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author HP
 */
int rename_this_file(const char *_from, const char *_to, int mode)
{
	char from[512]; strcpy(from, _from);
	char to[512]; strcpy(to, _to);
	char *file1 = strrchr(from, '/');
	char *file2 = strrchr(to,   '/');
	char *tag1 = NULL;
	char *tag2 = NULL;
	int ret = KW_SUCCESS;
	if (strcmp(file1, file2) == 0) {
		log_msg("%s mv OK",file1);
	} else {
		log_msg("%s diff %s",file1,file2);
		return -EPERM;
	}
	/** deprecated return rename_file(from,to); */
	/** @todo check move paths
	 * file can be moved only within correct folder domains
	 * e.g. within audio, within image, within files etc.
	 * moving between say audio and image should be RESTRICTED
	 */
	if (strcmp(strtok(from, "/"), strtok(to, "/")) != 0) {
		if (strstr(_to, "/harsh") != _to) {
			log_msg("DOMAIN of mv not same");
			return -EPERM;
		}
	}
	strcpy(from, _from); strcpy(to, _to);
	*file1 = '\0'; *file2 = '\0';
	file1 = strdup(file1 + 1);
	tag1 = strrchr(_from,'/'); tag2 = strrchr(_to,'/');
	*tag1 = '\0'; *tag2 = '\0';
	tag1 = strrchr(_from,'/')+1; tag2 = strrchr(_to,'/')+1;
	log_msg("tag %s in %s",file1, tag2);
	
	if (mode == DBFUSE_MV) {
		log_msg("untag %s from %s", file1, tag1);
		if (untag_file(tag1, file1) == KW_SUCCESS) {
			if (tag_file(tag2, file1) == KW_SUCCESS) {
				log_msg("tag operation successfull");
			} else {
				log_msg("tag operation failed");
				ret = KW_ERROR;
			}
			log_msg("mv operation successfull");
		} else {
			log_msg("mv operation failed");
			ret = KW_ERROR;
		}
	} else if (mode == DBFUSE_CP) {
		if (tag_file(tag2, file1) == KW_SUCCESS) {
			log_msg("tag operation successfull");
		} else {
			log_msg("tag operation failed");
			ret = KW_ERROR;
		}
		log_msg("cp operation successfull");
	}
	free(file1);
	return ret;
}

/**
 * @brief remove the said file
 * @param path
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author HP SG
 */
int remove_this_file(const char *path)
{
	
	char *filename = (char *)get_entry_name(path);
	char *tptr = filename -2;
	char *tagname = NULL;
	char *t2 = NULL;
	
	log_msg ("remove_this_file: %s",path);
	while (*tptr != '/') tptr--; /* seperat directory name */
	tptr++;
	
	tagname = malloc(filename - tptr);
	t2 = tagname;
	while (*tptr != '/') {
		*t2 = *tptr;
		t2++; tptr++;
	}
	*t2 = '\0';
	
	if (untag_file(tagname, filename) == KW_SUCCESS) {
		log_msg("remove_this_file: untag file successful");
		free(tagname);
		return KW_SUCCESS;
	}
	free(tagname);
	log_msg("remove_this_file: untag file failed");
	
	return KW_FAIL;
}

/**
 * @brief create a new directory
 * @param path
 * @param mode
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL, KW_ERROR: ERROR
 * @author HP SG
 */
int make_directory(const char *path, mode_t mode)
{
	char *newtag = NULL;
	char *tptr = NULL;
	char *parenttag = NULL;
	char *t2 = NULL;
	(void)mode;
	log_msg ("make_directory: %s",path);
	newtag = (char *)get_entry_name(path);
	
	if (add_tag(newtag,USER_MADE_TAG) != KW_SUCCESS) {
		log_msg ("make_directory: failed to add tag %s",newtag);
		return KW_FAIL;
	}
	
	tptr = newtag -2;
	while (*tptr != '/') tptr--;
	tptr++;
	
	parenttag = malloc(newtag - tptr);
	t2 = parenttag;
	while (*tptr != '/') {
		*t2 = *tptr;
		t2++; tptr++;
	}
	*t2 = '\0';
	
	if (add_association(newtag, parenttag, ASSOC_SUBGROUP) != KW_SUCCESS) {
		log_msg ("make_directory: failed to add association");
		return KW_FAIL;
	}
	
	log_msg ("make_directory: success");
	return KW_SUCCESS;
}

/**
 * @brief remove directory
 * @param path
 * @return KW_SUCCESS: SUCCESS, KW_FAIL: FAIL
 * @author HP
 */
int remove_directory(const char *path)
{
	log_msg ("remove_directory: %s",path);
	
	if (remove_tag(get_entry_name(path)) == KW_SUCCESS) {
		return KW_SUCCESS;
	}
	
	return KW_FAIL;
}
