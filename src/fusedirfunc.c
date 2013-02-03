/**
 * @file fusedirfunc.c
 * @brief fuse directory related operations
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
