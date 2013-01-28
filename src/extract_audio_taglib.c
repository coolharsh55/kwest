/* extract_audio_taglib.c
 * extract audio metadata using taglib
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

#include "extract_audio_taglib.h"
#include "magicstrings.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* metadata_audio_fields
 */
struct metadata audio_fields()
{
	struct metadata M;
	/* int i; */
	
	M.argc = 4;
	
	M.argv = malloc(M.argc * sizeof(char *));
	/* store character array of metadata * /
	/ * depreciated in favor of strdup * /
	M.argv[0] = malloc(sizeof(char) * strlen(TAG_ARTIST) + 1);
	M.argv[1] = malloc(sizeof(char) * strlen(TAG_ALBUM) + 1);
	M.argv[2] = malloc(sizeof(char) * strlen(TAG_TITLE) + 1);
	M.argv[3] = malloc(sizeof(char) * strlen(TAG_GENRE) + 1);
	
	/ * access through M.argv[i] * /
	strcpy(M.argv[0], TAG_ARTIST);
	strcpy(M.argv[1], TAG_ALBUM);
	strcpy(M.argv[2], TAG_TITLE);
	strcpy(M.argv[3], TAG_GENRE);
	*/
	
	M.argv[0] = strdup(TAG_ARTIST);
	M.argv[1] = strdup(TAG_ALBUM);
	M.argv[2] = strdup(TAG_TITLE);
	M.argv[3] = strdup(TAG_GENRE);
	
	return M;
}

/* format_string 
 * convert string to Title Case and trim Whitespaces
 * author : @RS @SG
 */
static char *format_string(char *string)
{
	int i;
	
	for(i = 0; string[i] != '\0'; i++) {
		
		if(i == 0 || !isalpha(string[i-1])) {
			string[i] = toupper(string[i]);
		} else {
			string[i] = tolower(string[i]);
		}
		if(string[i] == '/' || string[i] == '\\') {
		string[i] = ' ';
		}
	}
	
	/* Remove End of string White Spaces */
	while(string[--i]==' '); string[++i]='\0';
	
	return string;
}

/* extract_metadata_file: extract from physical location
 * param: const char* path - path of file
 * return: 0 SUCCESS, 1 NA, -1 ERROR
 * author: @HP
 * */
TagLib_File *extract_metadata_file(const char* path, struct metadata_audio *M)
{
	TagLib_File* file = taglib_file_new(path); 
	if(file == NULL) {
		/* log_error("is_audio","ERROR opening file"); */
		return NULL;
	}
	if(taglib_file_is_valid(file) == 0) {
		extract_clear_strings(file);
		return NULL;
	}
		
	TagLib_Tag* tag = taglib_file_tag(file);
	/* struct metadata_audio to store audio keywords */
	M->title = format_string(taglib_tag_title(tag));
	M->artist = format_string(taglib_tag_artist(tag));
	M->album = format_string(taglib_tag_album(tag));
	M->genre = format_string(taglib_tag_genre(tag));
	
	/* extract_clear_strings(file); */
	return file;
}

/* is_audio: check if filetype = audio
 * param: const char* path - path of file
 * return: 0 AUDIO, 1 NO, -1 ERROR
 * author: @HP
 * */
int is_audio(const char* path)
{
	TagLib_File* file = NULL;
	file = taglib_file_new(path);
	if(file == NULL) {
		/* log_error("is_audio","ERROR opening file"); */
		return -1;
	}
	if(taglib_file_is_valid(file) != 0) {
		taglib_file_free(file);
		return 0;
	} else {
		taglib_file_free(file);
		return 1;
	}
}

void extract_clear_strings(TagLib_File* file)
{
	taglib_tag_free_strings();
	taglib_file_free(file);
	return;
}
