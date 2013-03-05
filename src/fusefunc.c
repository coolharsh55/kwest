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
#include "apriori.h"
#include "dbinit.h"
#include "dbbasic.h"
#include "logging.h"
#include "flags.h"


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

	//char *tmp=strdup(path);
	char *pre;
	if(strlen(path)>10) {
		pre = strdup(strrchr(path,'/'));
		*(pre + 10) = '\0';
		log_msg("%s",(strcmp(pre,"/SUGGESTED" ) == 0)?"SUGGEST":"NOSUGGEST");
		if(strcmp(pre,"/SUGGESTED" ) == 0) {
			stbuf->st_mode= S_IFREG | KW_STFIL;
			return 0;
		}
		free(pre);
	}
	//free(tmp);

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
	char *suggest = NULL;
	char buffer[QUERY_SIZE];
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

	//log_msg("TAGS : %s",strrchr(path,'/') + 1);
	suggest = get_file_suggestions(strrchr(path,'/') + 1);

	if (suggest != NULL) {
		int i =0;
		char *entry = NULL;


		do {
			entry = get_token(suggest,i,',');
			if (strcmp(entry,"") == 0) {
				break;
			}
			strcpy(buffer, "SUGGESTED - ");
			strcat(buffer, entry);
			filler(buf, buffer, &st, 0);
			free(entry);
			i++;
		} while(entry != NULL);
	} else {
		//filler(buf, "NULL", &st, 0);
		log_msg("SUGGEST NULL");
	}
/*
	char *entry = suggest;
	char *putstring = NULL;

	do {
		strcpy(buffer, "SUGGESTED - ");
		int i =12;
		putstring = entry;
		entry = strchr(suggest, ',');
		if (entry == NULL) {
			filler(buf, "NULL", &st, 0);
			break;
		}
		do {
			buffer[i++] = *putstring;
			putstring++;
		} while(putstring != entry);
		buffer[i] = '\0';
		filler(buf, buffer, &st, 0);
		entry++;

	} while (entry != NULL); */
	//filler(buf,suggest, &st, 0);

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

	if(strstr(path, "SUGGESTED - ") == path) {
		//stbuf->st_mode= S_IFDIR | KW_STDIR;
		//stbuf->st_nlink=1;
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

/* DIRECTORY FUNCTIONS */


/**
 * @brief make directory
 * @param path
 * @param mode: mode of directory
 * @return 0: SUCCESS, -errno: error
 * @author HP
 */  
int kwest_mkdir(const char *path, mode_t mode)
{
	log_msg("mkdir: %s",path);

	if(check_path_validity(path) == KW_SUCCESS) {
		log_msg("PATH NOT VALID");
		return -ENOENT;
	}
	return make_directory(path, mode);
}


/**
 * @brief remove given directory
 * @param path
 * @return 0: SUCCESS, -errno: error
 * @author HP
 */
int kwest_rmdir(const char *path)
{
	log_msg("rmdir: %s",path);
	
	if(check_path_validity(path) != KW_SUCCESS) {
		log_msg("PATH NOT VALID");
		return -ENOENT;
	}
	
	return remove_directory(path);
}



/* FILE FUNCTIONS */

 
/**
 * @brief open a file for read/write operations
 * @param path
 * @param fi: fuse file handle
 * @return 0: SUCCESS, -errno: error
 * @author HP
 */
static int kwest_open(const char *path, struct fuse_file_info *fi)
{
	int res;
	const char *abspath = NULL;
	log_msg("open: %s",path);

	if(check_path_validity(path) != KW_SUCCESS) {
		log_msg("PATH NOT VALID");
		return -ENOENT;
	}
	
	abspath = get_absolute_path(path); /* get absolute path on disk */
	if(abspath == NULL) {
		log_msg("ABSOLUTE PATH ERROR");
		return -EIO;
	}
	
	res = open(abspath, fi->flags); /* open system call */
	if (res == -1) {
		log_msg("COULD NOT OPEN FILE");
		return -errno;
	}

	close(res);
	return 0;
}


/**
 * @brief called when last handle to file is closed
 * @param path
 * @param fi: fuse file handle
 * @return 0: SUCCESS, -errno: error
 * @author HP 
 */
static int kwest_release(const char *path, struct fuse_file_info *fi)
{
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
	log_msg("release: %s",path);
	
	return 0;
}


/**
 * @brief called when creating a new file
 * @param path
 * @param mode: file permissions and mode
 * @param dev
 * @return 0: SUCCESS, -errno: error
 * @author HP 
 */
static int kwest_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;
	const char *abspath = get_absolute_path(path);
	log_msg("mknod: %s",path);

	if(check_path_validity(path) != KW_SUCCESS) {
		log_msg("PATH NOT VALID");
		return -ENOENT;
	}	
	
	if(abspath == NULL) {
		log_msg("ABSOLUTE PATH ERROR");
		return -EIO;
	}

	if (S_ISREG(mode)) { /* check permissions */
		log_msg("MKNOD FILE MODE");
		res = open(abspath, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0) { /* call system calls to create file */
			res = close(res);
		}
	} else {
		if (S_ISFIFO(mode)) { /* file is a pipe */
			res = mkfifo(abspath, mode);
		} else { /* its a file */
			log_msg("FILE MODE PROGRAM");
			res = mknod(abspath, mode, rdev);
		}
	}
	if (res == -1) {
		return -errno;
	}

	return 0;
}


/**
 * @brief rename a file to specified name
 * @param from: original filename and path
 * @param to: specified name and path
 * @return 0: SUCCESS, -errno: error
 * @author HP
 */ 
static int kwest_rename(const char *from, const char *to)
{
	log_msg("rename: %s to %s",from,to);

	if(check_path_validity(from) != KW_SUCCESS) {
		log_msg("PATH NOT VALID");
		return -ENOENT;
	}
	
	return rename_this_file(from, to);
}


/**
 * @brief remove file entry from system
 * @param path
 * @return 0: SUCCESS, -errno: error
 * @author HP 
 */
static int kwest_unlink(const char *path)
{
	
	log_msg("unlink: %s",path);

	if(check_path_validity(path) != KW_SUCCESS) {
		log_msg("PATH NOT VALID");
		return -ENOENT;
	}
	
	return remove_this_file(path);
}


/**
 * @brief read specified bytes from file
 * @param path
 * @param buf: buffer to hold bytes
 * @param size: size of data to be read
 * @param offset: offset of last read
 * @param fi: fuse file handle
 * @return 0: SUCCESS, -errno: error
 * @author HP 
 */
static int kwest_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	int fd = 0;
	int res = 0;
	const char *abspath = NULL;
	
	log_msg ("read: %s",path);
	
	if(check_path_validity(path) != KW_SUCCESS) {
		log_msg("PATH NOT VALID");
		return -ENOENT;
	}
	
	abspath = get_absolute_path(path);
	if(abspath == NULL) {
		log_msg("ABSOLUTE PATH ERROR");
		return -EIO;
	}

	fd = open(abspath, O_RDONLY); /* open file for reading */
	if (fd == -1) {
		log_msg("COULD NOT OPEN FILE");
		return -errno;
	}

	res = pread(fd, buf, size, offset); /* pread doesn't lock file */
	if (res == -1) {
		log_msg("FILE READ ERROR");
		res = -errno;
	}

	close(fd);
	return res;
}


/**
 * @brief write specified bytes to file
 * @param path
 * @param buf: buffer to hold bytes
 * @param size: size of data to be written
 * @param offset: offset of last read
 * @param fi: fuse file handle
 * @return 0: SUCCESS, -errno: error
 * @author HP 
 */
static int kwest_write(const char *path, const char *buf, size_t size,
                       off_t offset, struct fuse_file_info *fi)
{
	int fd = 0;
	int res = 0;
	const char *abspath = NULL;
	
	log_msg ("write: %s",path);
	
	if(check_path_validity(path) != KW_SUCCESS) {
		log_msg("PATH NOT VALID");
		return -ENOENT;
	}
	
	abspath = get_absolute_path(path);
	if(abspath == NULL) {
		return -EIO;
	}

	fd = open(abspath, O_WRONLY); /* open file for writing */
	if (fd == -1) {
		return -errno;
	}
	
	res = pwrite(fd, buf, size, offset); /* pwrite system call */
	if (res == -1) {
		res = -errno;
	}

	close(fd);
	return res;
}


/**
 * @brief truncate file
 * @param path
 * @param size
 * @return 0: SUCCESS, -errno: error
 * @author HP 
 */
static int kwest_truncate(const char *path, off_t size)
{
	log_msg("truncate: %s", path);
	
	int res;

	const char *abspath = NULL;
	
	if(check_path_validity(path) != KW_SUCCESS) {
		log_msg("PATH NOT VALID");
		return -ENOENT;
	}
	
	abspath = get_absolute_path(path);
	if(abspath == NULL) {
		log_msg("ABSOLUTE PATH ERROR");
		return -EIO;
	}
	
	res = truncate(abspath, size);
	if (res == -1) {
		log_msg("TRUNCATE FILE ERROR");
		return -errno;
	}

	return 0;
}


/**
 * @brief chande file modes and permissions
 * @param path
 * @param mode: mode of file to be set
 * @return 0: SUCCESS, -errno: error
 * @author HP 
 */
static int kwest_chmod(const char *path, mode_t mode)
{
	int res;
	const char *abspath = NULL;
	
	log_msg ("chmod: %s",path);
	
	if(check_path_validity(path) != KW_SUCCESS) {
		log_msg("PATH NOT VALID");
		return -ENOENT;
	}
	
	abspath = get_absolute_path(path);
	if(abspath == NULL) {
		return -EIO;
	}
	/* get absolute path, pass to syscall chmod */
	res = chmod(abspath, mode);
	if (res == -1) {
		return -errno;
	}
	
	return 0;
}


/**
 * @brief change file owner and permissions
 * @param path
 * @param uid
 * @param gid
 * @return 0: SUCCESS, -errno: error
 * @author HP 
 */
static int kwest_chown(const char *path, uid_t uid, gid_t gid)
{
	int res;
	const char *abspath = NULL;
	
	log_msg ("chown: %s",path);
	
	if(check_path_validity(path) != KW_SUCCESS) {
		log_msg("PATH NOT VALID");
		return -ENOENT;
	}
	
	abspath = get_absolute_path(path);
	if(abspath == NULL) {
		return -EIO;
	}
	/* get absolute path, pass to syscall lchown */
	res = lchown(abspath, uid, gid);
	if (res == -1)
		return -errno;
	
	return 0;
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
