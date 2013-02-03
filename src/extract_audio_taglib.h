/**
 * @file extract_audio_taglib.h
 * @brief extract audio metadata using taglib
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

#ifndef EXTRACT_AUDIO_TAGLIB_H_INCLUDED
#define EXTRACT_AUDIO_TAGLIB_H_INCLUDED

#include <stdio.h>
#include <taglib/tag_c.h>


struct metadata_audio {
	const char *title;
	const char *artist;
	const char *album;
	const char *genre;
};

#ifndef METADATA_STRUCTURE
#define METADATA_STRUCTURE
	struct metadata 
	{
		int argc;
		char **argv;
	};
#endif

/*
 * Generalize metadata fields
 */
struct metadata audio_fields(void);

/*
 * Extract from physical location
 */
TagLib_File *extract_metadata_file(const char* path, 
                                   struct metadata_audio *M);


/*
 * Extract from fileid
 */
int extract_metadata_fileid(int file_id);

/*
 * Check if filetype = audio
 */
int is_audio(const char* path);

/*
 * Clear strings
 */
void extract_clear_strings(TagLib_File* file);

#endif
