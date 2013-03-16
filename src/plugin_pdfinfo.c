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
#include "logging.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

static int do_on_init(void *obj)
{
	(void)obj;
	printf("do_on_init\n");
	return KW_SUCCESS;
}

static int initializations(void)
{
	/* add plugin mime types to metadata table
	log_msg("plugin extractor has added database entries\n"); */
	printf("initializations\n");
	return KW_SUCCESS;
}

static int do_on_cleanup(struct kw_metadata *s)
{
	int i=0;
	char *memchar = NULL;
	printf("do_on_cleanup\n");
	if (s->type != NULL) {
		free(s->type);
	}
	if (s->obj != NULL) {
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
		//printf("%d:pdfinfo %s",++i,buffer);
		if (strstr(buffer, "Title:") == buffer) {
			//s->tagv[0] = strdup(buffer);
			printf("%d-%s@",strlen(buffer),buffer);
			format_string(buffer);
		} else if (strstr(buffer, "Subject:") == buffer) {
			//s->tagv[0] = strdup(buffer);
		} else if (strstr(buffer, "Author:") == buffer) {
			//s->tagv[0] = strdup(buffer);
		} else if(strstr(buffer, "Publisher:") == buffer) {
			//s->tagv[0] = strdup(buffer);
		}
		log_msg("%2d:%s",i++,buffer); 
	}
	
	if (i == 0) {
		return KW_ERROR;
	}
	
	if (pclose(pdfinfo) != 0) {
		printf(" Error: Failed to close command stream");
	}
	return KW_SUCCESS;
}

static bool is_of_type(const char *filepath)
{
	printf("is_of_type : %s\n",filepath);
	if (pipepdfinfo(filepath, NULL) == KW_SUCCESS) {
		printf("file has PDF metadata\n");
		return true;
	} else {
		printf("file does not have PDF metadata\n");
	}
	return false;
}

static int metadata_extract(const char *filename, struct kw_metadata *s)
{
	char *memchar = NULL;
	char buffer[1024];
	int ret = KW_ERROR;
	printf("metadata_extract\n");
	
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
	
	ret = pipepdfinfo(filename, s);
	/*
	memchar = strdup("test");
	memchar = format_string(memchar);
	s->tagv[0] = memchar;
	memchar = strdup("test");
	memchar  = format_string(memchar);
	s->tagv[1] = memchar;
	memchar = strdup("test");
	memchar = format_string(memchar);
	s->tagv[2] = memchar;
	memchar = strdup("test");
	memchar = format_string(memchar);
	s->tagv[3] = memchar;
	*/
	
	s->obj = NULL;
	s->do_init = &do_on_init;
	s->do_cleanup = &do_on_cleanup;
	
	return ret;
}

static int metadata_update(const char *filename, struct kw_metadata *s)
{
	printf("metadata_update\n");
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
	printf("EXPERIMENTAL\n");
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
		//plugin->thisplugin = this;
		plugin->is_of_type         = &is_of_type;
		plugin->p_metadata_extract = &metadata_extract;
		plugin->p_metadata_update  = &metadata_update;
		plugin->on_load            = &on_load;
		plugin->on_unload          = &on_unload;
	} else {
		
	}
	
	return plugin;
}
