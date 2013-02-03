/**
 * @file fusefilefunc.c
 * @brief fuse file related operations
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
