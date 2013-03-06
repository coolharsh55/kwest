/**
 * @file metadata_format.h
 * @brief metadata format structures for plugins
 * @author Harshvardhan Pandit
 * @date March 2013
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
 
#ifndef METADATA_FORMAT_H
#define METADATA_FORMAT_H

/** @struct kw_metadata 
 * generalized metadata structure 
 * @note all plugins must pass meta-information through this structure
 */
struct kw_metadata {
	/** type of metadata */
	char  *type; 
	/** number of tags in metadata */
	int tagc; 
	/** tag type array */
	char **tagtype; 
	/** tag value array */
	char **tagv; 
	/** object for use by plugin */
	void *obj; 
	/** @fn int (*do_init)(void *);  
	 * do_init initialization options upon extraction
	 * @param void * object of passing reference
	 * @return KW_SUCCESS on SUCCESS; KW_FAIL / KW_ERROR on FAIL/ERROR
	 * @see do_cleanup()
	 */
	int (*do_init)(void *); 
	/** @fn int (*do_cleanup)(struct kw_metadata *);  
	 * callback function for cleaning up metadata after metadata has been 
	 * extracted. 
	 * @warning The structure passed is a self-reference.
	 * @param kw_metadata struct self-reference
	 * @return KW_SUCCESS on SUCCESS; KW_FAIL / KW_ERROR on FAIL/ERROR
	 * @see do_init()
	 */
	int (*do_cleanup)(struct kw_metadata *); 
};

#endif
