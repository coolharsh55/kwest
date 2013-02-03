/**
 * @file export.c
 * @brief export kwest path & files to external location
 * @author Sahil Gupta
 * @date December 2012
 */

/* Execution : ./export tag_name absolute_path */

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

#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/sendfile.h>

#include "export.h"
#include "dbbasic.h"
#include "dbinit.h"
#include "flags.h"


/**
 * @brief Copies file from source to destination
 * @param source - source of file
 * @param destination - path where file is to be copied
 * @return KW_SUCCESS : SUCCESS, KW_FAIL : FAIL
 * @author SG
 */
static int send_file(const char *source,const char *destination)
{
	int read_src;
	int write_dst;
	struct stat stat_buf;
	off_t offset = 0;
	char dst[_POSIX_PATH_MAX];
	 
	/* Open the input file */
	read_src = open(source, O_RDONLY);
	if(read_src == -1) {
		printf("Cannot open source file : %s", source);
		return KW_FAIL;
	}
	 
	/* Call Stat on input file to obtain its size */
	if(fstat(read_src, &stat_buf) == -1) {
		printf("Error accessing source file : %s", source);
		return KW_FAIL;
	}
	
	/* Open the dstput file for writing, 
	 * with the same permissions as the source file 
	 */
	strcpy(dst,destination);
	strcat(dst,strrchr(source,'/'));
	write_dst = open(dst , O_WRONLY | O_CREAT, stat_buf.st_mode);
	if( write_dst == -1 ) {
		printf("Cannot open destination file : %s", dst);
		return KW_FAIL;
	}
	
	/* Blast the bytes from one file to the other */
	if(sendfile(write_dst, read_src, &offset, stat_buf.st_size) == -1){
		return KW_FAIL; 
	}
	
	/* Close Files */
	close (read_src);
	close (write_dst);
	 
	return KW_SUCCESS;
}

/**
 * @brief Exports a tag in kwest as Directory-file structure on File System
 * @param tag - Tag name in Kwest
 * @param path - Absolute Path of Location where tag is to be exported
 * @return KW_SUCCESS : SUCCESS, KW_FAIL : FAIL
 * @author SG
 */
int export(const char *tag,const char *path)
{
	DIR *directory;
	const char *filename,*tagname,*filepath;
	char tagpath[_POSIX_PATH_MAX];
	void *ptr;
	
	/* Check if path is valid */
	if((directory = opendir(path)) == NULL) {
		printf("Error opening path");
		return KW_FAIL;
	}
	if(closedir(directory) != 0){
		printf("Error closing path");
	}
	
	/* Form path for creating Directory */
	strcpy(tagpath,path);
	strcat(strcat(tagpath,"/"),tag);
	
	/* Create Directory for tag */ 
	printf("Creating Dir : %s",tag);
	if(mkdir(tagpath, KW_STDIR) == -1 && errno != EEXIST) {
		printf("Error Creating Directory");
		return KW_FAIL;
	} 
	
	/* Copy Files in Directory */
	ptr = get_fname_under_tag(tag); /* Get Path */
	while((filename = string_from_stmt(ptr))!= NULL) {
		filepath = get_abspath_by_fname(filename);
		if(send_file(filepath,tagpath) == KW_FAIL){ 
			printf("Error copying file %s",filename);
		} else {
			printf("Copy File : %s",filename);
		}
	}
	
	/* Copy Sub-Directories */
	ptr = get_tags_by_association(tag,ASSOC_SUBGROUP); /* Get Tag */
	while((tagname = string_from_stmt(ptr))!= NULL) {
		/* Copy Sub-Directory contents */
		export(tagname,tagpath); 
	}
	
	return KW_SUCCESS;
} 

/**
 * @brief Main function
 * @param argc
 * @param argv - [1]Tag name in Kwest, [2]Absolute Path where tag is exported
 * @return KW_SUCCESS : SUCCESS, KW_FAIL : FAIL
 * @author SG
 */
int main(int argc,const char *argv[])
{
	if(argc < 3) {
		return KW_FAIL;
	}
	
	if(export(argv[1],argv[2]) == KW_SUCCESS) {
		printf("Tag Exported");
		close_db();
		return KW_SUCCESS;
	}
	
	printf("Operation Failed");
	close_db(); 
	return KW_FAIL;
}
