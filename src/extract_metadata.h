/* audio.h
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

#ifndef KWEST_EXTRACT_METADATA_H
#define KWEST_EXTRACT_METADATA_H

#ifndef METADATA_STRUCTURE
#define METADATA_STRUCTURE
	struct metadata 
	{
		int fieldcount;
		char *mime;
		char **tagtype;
		char **tagvalue;
	};
#endif

int audio_metadata(void);

#endif
