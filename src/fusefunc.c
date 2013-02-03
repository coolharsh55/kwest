/**
 * @file fusefunc.c
 * @brief fuse functions implementations
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
 
#define FUSE_USE_VERSION 26

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fuse.h>

#include "fusefunc.h"
#include "dbfuse.h"
#include "dbinit.h"
#include "logging.h"
#include "flags.h"
#include "fusefilefunc.c"
#include "fusedirfunc.c"


/**
 * @brief get attributes for corresponding entry
 * @param path
 * @param stbuf: stat buffer pointer
 * @return 0: SUCCESS, -ENOENT: no_entry, -EIO: IOerror, -EACCES: no_acces
 * @author HP 
 */
static int kwest_getattr(const char *path, struct stat *stbuf)
{
	const char *abspath = NULL;
	log_msg("getattribute: %s",path);
	
	if(_is_path_root(path) == true) {
		log_msg("PATH IS ROOT");
		stbuf->st_mode= S_IFDIR | KW_STDIR;
		stbuf->st_nlink=1;
		return 0;
	}
	
	if(check_path_validity(path) != KW_SUCCESS) {
		log_msg("PATH NOT VALID");
		return -ENOENT;
	}
	
	if(path_is_dir(path) == true) {
		log_msg("PATH IS DIR");
		stbuf->st_mode= S_IFDIR | KW_STDIR;
		stbuf->st_nlink=1;
		return 0;
	} else if(path_is_file(path) == true) {
		log_msg("PATH IS FILE");
		abspath=get_absolute_path(path);
		stbuf->st_mode= S_IFREG | KW_STFIL;
		if(abspath == NULL) {
			return -EIO;
		}
		if(stat(abspath,stbuf) == 0) {
			free((char *)abspath);
			return 0;
		} else {
			log_msg("STAT ERROR");
			free((char *)abspath);
			return -EIO;
		}		
	}
	log_msg("ACCESS ERROR");
	return -EACCES;
}

/**
 * @brief list sub-directories and files
 * @param path
 * @param buf: buffer to store directory entries
 * @param filler: function to fill buffer with entry
 * @param offset: offset for last entry
 * @param fi: fuse struct for file info
 * @return 0: SUCCESS, -ENOENT: no_entry, -EIO: IOerror, -EACCES: no_acces
 * @author HP 
 */
static int kwest_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
	const char *direntry = NULL;
	void *ptr = NULL;
	struct stat st;
	log_msg("readdir: %s",path);
	
	/*
	if(check_path_validity(path) != KW_SUCCESS) {
		return -ENOENT;
	}
	*/
	memset(&st, 0, sizeof(st));
	st.st_mode = S_IFDIR | KW_STDIR;
	
	while((direntry = readdir_dirs(path, &ptr)) != NULL) {
		if (filler(buf, direntry, &st, 0) == 1) {
			break;
		}
	}
	
	direntry = NULL; ptr = NULL;
	memset(&st, 0, sizeof(st));
	st.st_mode = S_IFREG | KW_STFIL;
	
	while((direntry = readdir_files(path, &ptr)) != NULL) {
		if (filler(buf, direntry, &st, 0) == 1) {
			break;
		}
	}
	
	return 0;
}


/**
 * @brief checks if access is valid
 * @param path
 * @param mask
 * @return 0: SUCCESS, -ENOENT: no_entry, -EIO: IOerror, -EACCES: no_acces
 * @author HP
 */
static int kwest_access(const char *path, int mask)
{
	int res;
	const char *abspath = NULL;
	log_msg("access: %s",path);
	
	return 0; 
	/* for time being */
	
	if(check_path_validity(path) != KW_SUCCESS) {
		log_msg("PATH NOT VALID");
		return -ENOENT;
	}
	if(*(path + 1) == '\0') {
		return 0;
	}
	
	abspath = get_absolute_path(path);
	if(abspath == NULL) {
		log_msg("ABSOLUTE PATH ERROR");
		return -EIO;
	}
	
	res = access(path, mask);
	if (res == -1) {
		log_msg("ERROR ACCESSING FILE");
		return -errno;
	}
	
	return 0;
}


/**
 * @brief operations performed while unmount
 * @param private_data
 * @return void
 * @author HP
 */
void kwest_destroy(void *private_data)
{
	log_msg("filesytem is being unmounted...");
	close_db();
	log_close();
}


/* __FUSE FILESYSTEM OPERATIONS STRUCTURE__ */

/**
 * @brief fuse operations as functions
 * @note commented out part is not implemented
 */
static struct fuse_operations kwest_oper = {
/* BASIC FILESYSTEM OPERATIONS */ 
	.getattr	 = kwest_getattr,
	.readdir	 = kwest_readdir,
	.access		 = kwest_access,
	.truncate	 = kwest_truncate, 
	.destroy	 = kwest_destroy,

/* FILE RELATED FILESYSTEM OPERATIONS */
	.open		= kwest_open,
	.release	= kwest_release,
	.mknod		= kwest_mknod,
	.rename		= kwest_rename,
	.unlink		= kwest_unlink,
	.read		= kwest_read,
	.write		= kwest_write,
	.chmod		= kwest_chmod,
	.chown		= kwest_chown,

/* DIRECTORY RELATED FILESYSTEM OPERATIONS */
	.mkdir		= kwest_mkdir,
	.rmdir		= kwest_rmdir,

/* NOT IMPLEMENTED */
/*	.symlink	= kwest_symlink, */
/*	.readlink	= kwest_readlink, */
/*	.link		= kwest_link, */
/*	.utimens	= kwest_utimens, */
/*	.statfs		= kwest_statfs, */
/*	.fsync		= kwest_fsync, */

/*
#ifdef HAVE_SETXATTR
	.setxattr	= kwest_setxattr,
	.getxattr	= kwest_getxattr,
	.listxattr	= kwest_listxattr,
	.removexattr	= kwest_removexattr,
#endif
*/
};


/**
 * @brief pass control to fuse daemon
 * @param argc
 * @param argv
 * @return 0: SUCCESS, -errno: error
 * @author HP
 */ 
int call_fuse_daemon(int argc, char **argv)
{
	return(fuse_main(argc, argv, &kwest_oper, NULL));
}
