/**
 * @file dbplugin.c
 * @brief Wrapper functions for interaction with database
 * @author Sahil Gupta
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dbplugin.h"
#include "dbinit.h"
#include "dbbasic.h"
#include "flags.h"
#include "magicstrings.h"

/**
 * @brief Add new mime type to Kwest
 * @param mime Mime Type
 * @return void
 * @author SG
 */
void add_mime_type(char *mime)
{
	/*! char query[QUERY_SIZE]; */

	/* Create new table for new mime type */
	/*!
	sprintf(query, "create table if not exists %s (fno integer);", mime);
	sqlite3_exec(get_kwdb(),query,0,0,0);
	*/

	add_tag(mime, SYSTEM_TAG);
	add_association(mime, TAG_ROOT, ASSOC_SUBGROUP);
}

/**
 * @brief Add new metadata type to existing mime type
 * @param mime Mime Type
 * @param metadata New Metadata to be added
 * @return void
 * @author SG
 */
void add_metadata_type(char *mime, char *metadata)
{
	/*! char query[QUERY_SIZE]; */

	/* Add new metadata to existing mime table */
	/*!
	sprintf(query, "alter table %s add column %s text", mime, metadata);
	sqlite3_exec(get_kwdb(),query,0,0,0);
	*/

	add_tag(metadata, SYSTEM_TAG);
	add_meta_info(mime, metadata);
	add_association(metadata, mime, ASSOC_SUBGROUP);
}

/**
 * @brief Assign appropriate name if tag empty
 * @param type - metadata category
 * @param tag - metadata
 * @return tagname
 * @author SG
 */
static char *analyze_tag(const char *tag,const char *type)
{
	char *newtag = NULL;
	int taglength = 0;

	if( (tag == NULL) || ((strcmp(tag,"") == 0) || (strcmp(type,tag)==0)
	                  || (strstr(tag,TAG_UNKNOWN) != NULL))) {
		taglength = strlen(TAG_UNKNOWN) + strlen(type) + 2;
		newtag = (char *)malloc(taglength * sizeof(char));
		newtag = strcpy(newtag, TAG_UNKNOWN);
		newtag = strcat(newtag, " ");
		newtag = strcat(newtag, type);
	}

	return newtag;
}

/**
 * @brief Form association for metadata in file
 * @param mime Mime Type
 * @param tagname Metadata
 * @param fname File name
 * @return void
 * @author SG
 */
void associate_file_metadata(const char *mime,const char *tagname,
                             const char *fname)
{
	char *newtag=NULL;

	/* No meta information */
	if((newtag = analyze_tag(tagname,mime)) != NULL) {
		/* Create Tag Unknown */
		add_tag(newtag,SYSTEM_TAG);
		/* Associate Tag Unknown with File Type*/
		add_association(newtag,mime,ASSOC_SUBGROUP);
		/* Tag File to Metadata Tag */
		tag_file(newtag,fname);
		free((char *)newtag);
	} else  /* Metadata Exist */ {
		/* Create Tag for Metadata */
		add_tag(tagname,USER_TAG);
		/* Associate Metadata tag with File Type */
		add_association(tagname,mime,ASSOC_SUBGROUP);
		/* Tag File to Metadata Tag */
		tag_file(tagname,fname);
	}
}

/**
 * @brief Form recursive association between tags to display metadata
 * @param mime - metadata category
 * @param tagname - metadata
 * @param parentmime - metadata category
 * @param tagname - metadata
 * @return void
 * @author SG
 */
void associate_tag_metadata(const char *mime,const char *tagname,
                            const char *parentmime,const char *parent)
{
	char *newtag,*parenttag;

	if((newtag = analyze_tag(tagname,mime)) != NULL) {
		/* Create Tag Unknown */
		add_tag(newtag,SYSTEM_TAG);
		/* Associate Tag Unknown with File Type*/
		add_association(newtag,mime,ASSOC_SUBGROUP);
		/* Tag File to Metadata Tag */
		if((parenttag = analyze_tag(parent,parentmime)) != NULL) {
			add_association(parenttag,newtag,ASSOC_SUBGROUP);
			free((char *)parenttag);
		} else {
			add_association(parent,newtag,ASSOC_SUBGROUP);
		}
		free((char *)newtag);
	} else { /* Metadata Exist */
		/* Create Tag for Metadata */
		add_tag(tagname,USER_TAG);
		/* Associate Metadata tag with File Type */
		add_association(tagname,mime,ASSOC_SUBGROUP);
		/* Tag File to Metadata Tag */
		if((parenttag = analyze_tag(parent,parentmime)) != NULL) {
			add_association(parenttag,tagname,ASSOC_SUBGROUP);
			free((char *)parenttag);
		} else {
			add_association(parent,tagname,ASSOC_SUBGROUP);
		}
	}
}
