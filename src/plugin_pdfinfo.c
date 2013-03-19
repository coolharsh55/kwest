/**
 * @file plugin_extractor.c
 * @brief extract command wrapper for files
 * @author Harshvardhan Pandit
 * @date March 2013
 */
 
#include "plugins_extraction.h"
#include "plugin_pdfinfo.h"
#include "flags.h"
#include "dbbasic.h"
#include "dbplugin.h"
#include "logging.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define TAG_PDF "PDF"
#define TAG_PDF_TITLE "Title"
#define TAG_PDF_SUBJECT "Subject"
#define TAG_PDF_AUTHOR "Author"
#define TAG_PDF_PUBLISHER "Publisher"

static int do_on_init(void *obj)
{
	(void)obj;
	return KW_SUCCESS;
}

static int initializations(void)
{
	add_mime_type(TAG_PDF);
	add_metadata_type(TAG_PDF, TAG_PDF_TITLE);
	add_metadata_type(TAG_PDF, TAG_PDF_SUBJECT);
	add_metadata_type(TAG_PDF, TAG_PDF_AUTHOR);
	add_metadata_type(TAG_PDF, TAG_PDF_PUBLISHER);
	log_msg("plugin pdfinfo has added database entries\n");
	return KW_SUCCESS;
}

static int do_on_cleanup(struct kw_metadata *s)
{
	int i=0;
	char *memchar = NULL;
	if (s->type != NULL) {
		free(s->type);
	}
	if (s->obj != NULL) {
		associate_file_metadata(TAG_PDF_TITLE,s->tagv[0],s->obj);
		associate_file_metadata(TAG_PDF_SUBJECT,s->tagv[1],s->obj);
		associate_file_metadata(TAG_PDF_AUTHOR,s->tagv[2],s->obj);
		associate_file_metadata(TAG_PDF_PUBLISHER,s->tagv[3],s->obj);
	}
	if (s->tagc > 0) {
		for(i=0 ; i<s->tagc ; i++) {
			memchar = s->tagtype[i];
			if (memchar != NULL) {
				free(memchar);
			}
			memchar = s->tagv[i];
			if (memchar != NULL) {
				free(memchar);
			}
		}
		
		free(s->tagtype);
		free(s->tagv);
	}
	return KW_SUCCESS;
}


static char *format_string(char *string)
{
	int i;
	char *strptr = string;
	char *cleanend = NULL;
	if (string == NULL || string[0] == '\0') {
		return string;
	}
	
	while (*strptr != ':') {
		strptr++; 
	} strptr++;
	while (*strptr == ' ' || *strptr == '\t') {
		strptr++;
	}
	cleanend = strptr;
	while (*cleanend != '\0') {
		cleanend++; 
	} cleanend--;
	while (*cleanend == ' ' || *cleanend == '\t' || *cleanend == '\n')  {
		cleanend--;
	}
	cleanend++;
	*cleanend = '\0';

	strptr[0] = (char)toupper((int)strptr[0]);
	
	for (i = 1; strptr[i] != '\0'; i++) {
		if (!isalpha(strptr[i-1])) {
			strptr[i] = (char)toupper((int)strptr[i]);
		} else {
			strptr[i] = (char)tolower((int)strptr[i]);
		}
		if (strptr[i] == '/' || strptr[i] == '\\') {
		strptr[i] = ' ';
		}
	}

	return strptr;
}

static int pipepdfinfo(const char *filename, struct kw_metadata *s)
{
	char buffer[512];
	char command[1024];
	FILE *pdfinfo = NULL;
	int i=0;
	sprintf(command, "pdfinfo %s", filename);
	pdfinfo = popen(command, "r");
	if(pdfinfo == NULL){
		log_msg("Could not open pipe for output");
		return KW_ERROR;
	}
	/* kw_metadata == NULL -> is_of_type */
	if (s == NULL) {
		if (fgets(buffer, 512 , pdfinfo) == NULL) {
			return KW_ERROR;
		} else {
			return KW_SUCCESS;
		}
	}
	/* else extract metadata into structure */
	while(fgets(buffer, 512 , pdfinfo) != NULL) {
		if (strstr(buffer, "Title:") == buffer) {
			s->tagv[0] = strdup(format_string(buffer));
		} else if (strstr(buffer, "Subject:") == buffer) {
			s->tagv[1] = strdup(format_string(buffer));
		} else if (strstr(buffer, "Author:") == buffer) {
			s->tagv[2] = strdup(format_string(buffer));
		} else if(strstr(buffer, "Publisher:") == buffer) {
			s->tagv[3] = strdup(format_string(buffer));
		}
	}
	
	if (i == 0) {
		return KW_ERROR;
	}
	
	if (pclose(pdfinfo) != 0) {
		log_msg(" Error: Failed to close command stream");
	}
	return KW_SUCCESS;
}

static bool is_of_type(const char *filepath)
{
	if (pipepdfinfo(filepath, NULL) == KW_SUCCESS) {
		return true;
	} else {
	}
	return false;
}

static int metadata_extract(const char *filename, struct kw_metadata *s)
{
	char *memchar = NULL;

	s->obj = NULL;
	s->do_cleanup = &do_on_cleanup;
	
	s->type = strdup("PDF");
	s->tagc = 4;
	s->tagtype = (char **)malloc(4 * sizeof(char *));
	s->tagv = (char **)malloc(4 * sizeof(char *));
	
	memchar = strdup("title");
	s->tagtype[0] = memchar;
	s->tagv[0] = NULL;
	memchar = strdup("subject");
	s->tagtype[1] = memchar;
	s->tagv[1] = NULL;
	memchar = strdup("author");
	s->tagtype[2] = memchar;
	s->tagv[2] = NULL;
	memchar = strdup("publisher");
	s->tagtype[3] = memchar;
	s->tagv[3] = NULL;
	
	pipepdfinfo(filename, s);

	s->obj = NULL;
	s->do_init = &do_on_init;
	s->do_cleanup = &do_on_cleanup;
	
	return KW_SUCCESS;
}

static int metadata_update(const char *filename, struct kw_metadata *s)
{
	(void)s;
	if (!is_of_type(filename)) {
		return KW_ERROR;
	}
	return KW_SUCCESS;
}

static int on_load(struct plugin_extraction_entry *plugin)
{
	printf("plugin loaded\n");
	printf("name: %s\n", plugin->name);
	printf("type: %s\n", plugin->type);
	initializations();
	return KW_SUCCESS;
}
	
static int on_unload(struct plugin_extraction_entry *plugin)
{
	log_msg("plugin on unload\n");
	log_msg("freeing name: %s\n", plugin->name);
	free(plugin->name);
	log_msg("freeing type: %s\n", plugin->type);
	free(plugin->type);
	if (plugin->obj != NULL) {
		free(plugin->obj);
	}
	return KW_SUCCESS;
}

struct plugin_extraction_entry *load_pdfinfo_plugin()
{
	static struct plugin_extraction_entry *plugin = NULL;
	if (plugin == NULL) {
		plugin = (struct plugin_extraction_entry *) 
		         malloc(sizeof(struct plugin_extraction_entry));
		plugin->name = strdup("pdfinfo");
		plugin->type = strdup("PDF");
		plugin->obj  = NULL;
		plugin->is_of_type         = &is_of_type;
		plugin->p_metadata_extract = &metadata_extract;
		plugin->p_metadata_update  = &metadata_update;
		plugin->on_load            = &on_load;
		plugin->on_unload          = &on_unload;
	} else {
		
	}
	
	return plugin;
}
