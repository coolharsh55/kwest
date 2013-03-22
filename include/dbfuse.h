/**
 * @file dbfuse.h
 * @brief functions connecting fuse and database modules
 * @author Harshvardhan Pandit
 * @date December 2012
 */

/* LICENSE
 * Copyright 2013 Harshvardhan pandit
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
 
#ifndef KWEST_DBFUSE_H
#define KWEST_DBFUSE_H

#include <sys/stat.h>
#include "flags.h"

#define DBFUSE_CP 111
#define DBFUSE_MV 121

/*
 * checks whether given path is ROOT
 */
bool _is_path_root(const char *path);

/*
 * checks whether current path is valid in database
 */
int check_path_validity(const char *path);

int check_path_tags_validity(const char *path);
/*
 * checks whether given path has a directory entry
 */
bool path_is_dir(const char *path);

/*
 * checks whether given path has a file entry
 */
bool path_is_file(const char *path);

/*
 * returns absolute path of said file
 */
const char *get_absolute_path(const char *path);

/*
 * get directory entries for said path
 */
char *readdir_dirs(const char *path, void **ptr);

/*
 * get file entries for said path
 */
char *readdir_files(const char *path, void **ptr);

/*
 * create a new file and return is absolute path
 */
const char *get_newfile_path(const char *path);

/*
 * rename the said file from -> to
 */
int rename_this_file(const char *from, const char *to, int mode);

/*
 * remove the said file
 */
int remove_this_file(const char *path);

/*
 * create a new directory
 */
int make_directory(const char *path, mode_t mode);

/*
 * remove directory
 */
int remove_directory(const char *path);

 #endif
