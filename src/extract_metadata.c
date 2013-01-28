/* audio.c
 * metadata and relations associated with audio metadata
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

#include "extract_metadata.h"
#include "dbbasic.h"
#include "logging.h"
#include "flags.h"
#include "magicstrings.h"


/* is this really necessary to put in this file? */
int audio_metadata()
{
	/* Create tags to browse Audio Files */
	add_tag(TAG_AUDIO, SYSTEM_TAG);
	add_tag(TAG_ARTIST, SYSTEM_TAG);
	add_tag(TAG_ALBUM, SYSTEM_TAG);
	add_tag(TAG_GENRE, SYSTEM_TAG);
	
	/* Metadata available for Audio Files */
	add_meta_info(TAG_AUDIO, TAG_TITLE);
	add_meta_info(TAG_AUDIO, TAG_ARTIST);
	add_meta_info(TAG_AUDIO, TAG_ALBUM);
	add_meta_info(TAG_AUDIO, TAG_GENRE);
	
	/* Establish tag-tag associations */
	add_association(TAG_AUDIO, TAG_ROOT, ASSOC_SUBGROUP);
	add_association(TAG_ARTIST, TAG_AUDIO, ASSOC_SUBGROUP);
	add_association(TAG_ALBUM, TAG_AUDIO, ASSOC_SUBGROUP);
	add_association(TAG_GENRE, TAG_AUDIO, ASSOC_SUBGROUP);
	
	return KW_SUCCESS;
}


