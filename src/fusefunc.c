/**
 * @file fusefunc.c
 * @brief fuse functions implementations
 * @author Harshvardhan Pandit
 * @date March 2013
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
#include "dbapriori.h"
#include "apriori.h"
#include "dbinit.h"
#include "dbbasic.h"
#include "logging.h"
#include "flags.h"


/**
 * @fn static int kwest_getattr(const char *path, struct stat *stbuf)
 * @brief get attributes for corresponding entry
 * @param path file system path
 * @param stbuf stat buffer pointer
 * @note path is absolute to the file system
 * @return 0 on SUCCESS
 * @return -ENOENT on no_entry
 * @return -EIO on IOerror
 * @return -EACCES: no_acces
 * @author Harshvardhan Pandit
 * @see kwest_readdir
 * @ee kwest_access
 */
static int kwest_getattr(const char *path, struct stat *stbuf)
{
	const char *abspath = NULL;
	log_msg("getattribute: %s",path);
	/** check if path is root */
	if(_is_path_root(path) == true) {
		/*log_msg("PATH IS ROOT");*/
		stbuf->st_mode= S_IFDIR | KW_STDIR;
		stbuf->st_nlink=1;
		return 0;
	}
	/** check is path is a virtual suggestion */
	char *pre;
	if(strlen(path)>10) {
		pre = strdup(strrchr(path,'/'));
		*(pre + 10) = '\0';
		log_msg("%s",(strcmp(pre,"/SUGGESTED" ) == 0)?"SUGGESTED":"NOSUGGEST");
		if(strcmp(pre,"/SUGGESTED" ) == 0) {
			stbuf->st_mode= S_IFREG | KW_STFIL;
			return 0;
		}
		free(pre);
	}
	/** check if path is valid for kwest */
	if(check_path_validity(path) != KW_SUCCESS) {
		log_msg("PATH NOT VALID");
		return -ENOENT;
	}
	/** check if path is for a directory */
	if(path_is_dir(path) == true) {
		/*log_msg("PATH IS DIR");*/
		stbuf->st_mode= S_IFDIR | KW_STDIR;
		stbuf->st_nlink=1;
		return 0;
	/** check if path is for a file */
	} else if(path_is_file(path) == true) {
		/*log_msg("PATH IS FILE");*/
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
 * @fn static int kwest_readdir(const char *path, void *buf,
 *            fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
 * @brief list sub-directories and files
 * @param path path file system path
 * @param buf buffer to store directory entries
 * @param filler function to fill buffer with entry
 * @param offset offset for last entry
 * @param fi fuse struct for file info
 * @note path is relative to file system
 * @return 0 on  SUCCESS
 * @return -ENOENT on no_entry
 * @return -EIO on IOerror
 * @return -EACCES on no_acces
 * @author Harshvardhan Pandit
 */
static int kwest_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                         off_t offset, struct fuse_file_info *fi)
{
	(void)offset;
	(void)fi;
	const char *direntry = NULL;
	char *suggest = NULL;
	char buffer[QUERY_SIZE];
	void *ptr = NULL;
	struct stat st;
	log_msg("readdir: %s",path);

	/** @todo
	 * check_path_validity(path)
	 * that recognises a valid kwest path
	 */

	/*
	if(check_path_validity(path) != KW_SUCCESS) {
		return -ENOENT;
	}
	*/

	/** @note FILLER fuse provides a buffer which is supposed to be
	 * filled with entries (listings) for the current readdir command
	 * the function filler is provided by fuse
	 */
	memset(&st, 0, sizeof(st));
	st.st_mode = S_IFDIR | KW_STDIR;

	/** get directories under current path */
	while((direntry = readdir_dirs(path, &ptr)) != NULL) {
		if (filler(buf, direntry, &st, 0) == 1) {
			break;
		}
	}

	direntry = NULL; ptr = NULL;
	memset(&st, 0, sizeof(st));
	st.st_mode = S_IFREG | KW_STFIL;
	/** get files under current path */
	while((direntry = readdir_files(path, &ptr)) != NULL) {
		if (filler(buf, direntry, &st, 0) == 1) {
			break;
		}
	}

	/** get File suggestions under current path */
	suggest = get_file_suggestions(strrchr(path,'/') + 1);
	if (suggest != NULL) {
		int i = 0;
		char *entry;
		entry = (char *)malloc(strlen(suggest) * sizeof(char));
		do {
			get_token(&entry, suggest, i, ',');
			if (strcmp(entry, "") == 0) {
				break;
			}
			strcpy(buffer, "SUGGESTEDFILE - ");
			strcat(buffer, entry);
			filler(buf, buffer, &st, 0);
			i++;
		} while(1);
		free((char *) entry);
		free((char *) suggest);
	}

	/** get Tag suggestions under current path */
	suggest = get_tag_suggestions(strrchr(path,'/') + 1);
	if (suggest != NULL) {
		int i = 0;
		char *entry;
		entry = (char *)malloc(strlen(suggest) * sizeof(char));
		do {
			get_token(&entry, suggest, i, ',');
			if (strcmp(entry, "") == 0) {
				break;
			}
			strcpy(buffer, "SUGGESTEDTAG - ");
			strcat(buffer, entry);
			filler(buf, buffer, &st, 0);
			i++;
		} while(1);
		free((char *) entry);
		free((char *) suggest);
	}

	return 0;
}


/**
 * @fn static int kwest_access(const char *path, int mask)
 * @brief checks if access is valid
 * @param path path of file system
 * @param mask masking mode
 * @return 0 on SUCCESS
 * @return -ENOENT on no_entry
 * @return -EIO on IOerror
 * @return -EACCES on no_access
 * @author Harshvardhan Pandit
 */
static int kwest_access(const char *path, int mask)
{
	int res;
	const char *abspath = NULL;
	log_msg("access: %s",path);

	/** @todo
	 * kwest_access: check proper access rights
	 */

	/** @warning
	 * temporary return 0
	 */
	return 0;

	if(check_path_validity(path) != KW_SUCCESS) {
		log_msg("PATH NOT VALID");
		return -ENOENT;
	}
	if(*(path + 1) == '\0') {
		return 0;
	}

	if(strstr(path, "SUGGESTED - ") == path) {
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
 * @fn void kwest_destroy(void *private_data)
 * @brief operations performed while unmount
 * @param private_data abstract data pointer
 * @return void nothing
 * @see close_db
 * @see log_close
 * @note all unfreed memory must be freed here
 * @warning this function gets called explicitly only on "fusermount"
 * @author Harshvardhan Pandit
 */
void kwest_destroy(void *private_data)
{
	(void)private_data;
	log_msg("filesytem is being unmounted...");
	close_db();
	log_close();
}

/* DIRECTORY FUNCTIONS */


/**
 * @fn int kwest_mkdir(const char *path, mode_t mode)
 * @brief make directory
 * @details this is the normal mkdir system call. calling this is supposed to
 * add a tag by the given name. mkdir is intepreted as make-tag. Since tags
 * are represented as virtual directories, this is used to create them.
 * The contents of directories are displayed using the readdir operation
 * @param path path of file system
 * @param mode mode of directory
 * @return 0 on SUCCESS
 * @return -errno on error
 * @see kwest_readdir
 * @author Harshvardhan Pandit
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
 * @fn int kwest_rmdir(const char *path)
 * @brief remove given directory
 * @details this is the normal rmdir system call. calling this is supposed to
 * remove a tag by the given name. rmdir is intepreted as remove-tag.
 * @param path path of file system
 * @return 0 on SUCCESS
 * @return -errno on error
 * @author Harshvardhan Pandit
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

static char *get_cp_path()
{
	static char cppath[QUERY_SIZE];
	return cppath;
}

/**
 * @fn static int kwest_open(const char *path, struct fuse_file_info *fi)
 * @brief open a file for read/write operations
 * @param path path of file system
 * @param fi fuse file handle
 * @return 0 on SUCCESS
 * @return -errno on error
 * @author Harshvardhan Pandit
 */
static int kwest_open(const char *path, struct fuse_file_info *fi)
{
	int res;
	const char *abspath = NULL;
	log_msg("open: %s",path);

	char *cppath = get_cp_path();
	cppath = strcpy(cppath, path);
	log_msg("cppath: %s",cppath);
	
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
 * @fn static int kwest_release(const char *path, struct fuse_file_info *fi)
 * @brief called when last handle to file is closed
 * @param path path of file system
 * @param fi fuse file handle
 * @return 0 on SUCCESS
 * @return -errno on  error
 * @author Harshvardhan Pandit
 */
static int kwest_release(const char *path, struct fuse_file_info *fi)
{
	(void)fi;
	/* Just a stub.	 This method is optional and can safely be left
	   unimplemented */
	log_msg("release: %s",path);
	char *cppath = get_cp_path();
	if (*cppath != '$') {
		log_msg("cppath: %s",cppath);
		*cppath = '$';
	}
	return 0;
}


/**
 * @fn static int kwest_mknod(const char *path, mode_t mode, dev_t rdev)
 * @brief called when creating a new file
 * @details mknod functionality when called from out of the file system
 * by and external entity is not yet determined
 * @param path path of file system
 * @param mode file permissions and mode
 * @param dev creation mode
 * @return 0 on SUCCESS
 * @return -errno on error
 * @author Harshvardhan Pandit
 */
static int kwest_mknod(const char *path, mode_t mode, dev_t rdev)
{
	/** @todo
	 * kwest_mknod: create new file directly into file system?
	 */
	/** @todo
	 * kwest_mknod: what happens when utilities such as the browser want
	 * to create temporar files to work with cp/mv
	 */

	/** @bug
	 * cp/mv does not work correctly due to improper implementation of
	 * kwest_mknod. cp/mv will work correctly when it is from kwest
	 * to any other filesystem. internal working is buggy / incorrect.
	 */
	int res;
	const char *abspath = get_absolute_path(path);
	log_msg("mknod: %s",path);

	if(check_path_tags_validity(path) != KW_SUCCESS) {
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
 * @fn static int kwest_rename(const char *from, const char *to)
 * @brief rename a file to specified name
 * @param from original filename and path
 * @param to specified name and path
 * @return 0 on SUCCESS
 * @return -errno on error
 * @author Harshvardhan Pandit
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
 * @fn static int kwest_unlink(const char *path)
 * @brief remove file entry from system
 * @param path path of file system
 * @return 0 on SUCCESS
 * @return -errno on error
 * @author Harshvardhan Pandit
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
 * @fn static int kwest_read(const char *path, char *buf, size_t size,
 *                           off_t offset, struct fuse_file_info *fi)
 * @brief read specified bytes from file
 * @param path path of file system
 * @param buf buffer to hold bytes
 * @param size size of data to be read
 * @param offset offset of last read
 * @param fi fuse file handle
 * @return 0 on SUCCESS
 * @return -errno on error
 * @author Harshvardhan Pandit
 */
static int kwest_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	(void)fi;
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
 * @fn static int kwest_write(const char *path, const char *buf, size_t size,
                       off_t offset, struct fuse_file_info *fi)
 * @brief write specified bytes to file
 * @param path path of file system
 * @param buf buffer to hold bytes
 * @param size size of data to be written
 * @param offset offset of last read
 * @param fi fuse file handle
 * @return 0 on SUCCESS
 * @return -errno on error
 * @author Harshvardhan Pandit
 */
static int kwest_write(const char *path, const char *buf, size_t size,
                       off_t offset, struct fuse_file_info *fi)
{
	(void)fi;
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
 * @fn static int kwest_truncate(const char *path, off_t size)
 * @brief truncate file
 * @param path path of file system
 * @param size truncare length
 * @return 0 on SUCCESS
 * @return -errno on error
 * @author Harshvardhan Pandit
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
 * @fn static int kwest_chmod(const char *path, mode_t mode)
 * @brief chande file modes and permissions
 * @param path path of file system
 * @param mode mode of file to be set
 * @return 0 on SUCCESS
 * @return -errno on error
 * @author Harshvardhan Pandit
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
 * @fn static int kwest_chown(const char *path, uid_t uid, gid_t gid)
 * @brief change file owner and permissions
 * @param path path of file system
 * @param uid user id
 * @param gid group id
 * @return 0 on SUCCESS
 * @return -errno on error
 * @author Harshvardhan Pandit
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
 * @struct kwest_oper
 * @brief fuse operations as functions
 * @note commented out part is not implemented
 * @details implemented operations:
 @code
  	.getattr	 = kwest_getattr,
	.readdir	 = kwest_readdir,
	.access		 = kwest_access,
	.truncate	 = kwest_truncate,
	.destroy	 = kwest_destroy,

FILE RELATED FILESYSTEM OPERATIONS
	.open		= kwest_open,
	.release	= kwest_release,
	.mknod		= kwest_mknod,
	.rename		= kwest_rename,
	.unlink		= kwest_unlink,
	.read		= kwest_read,
	.write		= kwest_write,
	.chmod		= kwest_chmod,
	.chown		= kwest_chown,

DIRECTORY RELATED FILESYSTEM OPERATIONS
	.mkdir		= kwest_mkdir,
	.rmdir		= kwest_rmdir,

NOT IMPLEMENTED
	.symlink	= kwest_symlink,
	.readlink	= kwest_readlink,
	.link		= kwest_link,
	.utimens	= kwest_utimens,
	.statfs		= kwest_statfs,
	.fsync		= kwest_fsync,

#ifdef HAVE_SETXATTR
	.setxattr	= kwest_setxattr,
	.getxattr	= kwest_getxattr,
	.listxattr	= kwest_listxattr,
	.removexattr	= kwest_removexattr,
#endif
@endcode
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
 * @fn int call_fuse_daemon(int argc, char **argv)
 * @brief pass control to fuse daemon
 * @param argc argument count from main
 * @param argv argument values from main
 * @return 0 on SUCCESS
 * @return -errno on error
 * @author Harshvardhan Pandit
 */
int call_fuse_daemon(int argc, char **argv)
{
	return(fuse_main(argc, argv, &kwest_oper, NULL));
}
