/**
 * @file plugin_extractor.c
 * @brief extract command wrapper for files
 * @author Harshvardhan Pandit
 * @date March 2013
 */
 
#include "plugins_extraction.h"
#include "plugin_extract.h"
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
	return KW_SUCCESS;
}

static int initializations(void)
{
	/* add plugin mime types to metadata table
	log_msg("plugin extractor has added database entries\n"); */
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
	
	if (string == NULL || string[0] == '\0') {
		return string;
	}
	string[0] = (char)toupper((int)string[0]);
	
	for (i = 1; string[i] != '\0'; i++) {
		if (!isalpha(string[i-1])) {
			string[i] = (char)toupper((int)string[i]);
		} else {
			string[i] = (char)tolower((int)string[i]);
		}
		if (string[i] == '/' || string[i] == '\\') {
		string[i] = ' ';
		}
	}
	/* Remove End of string White Spaces */
	while (i-- > 0 && string[i] == ' '); string[++i]='\0';
	
	return string;
}

static int pipepdfinfo(const char *filename, char *buffer)
{
	char command[20];
	FILE *pdfinfo = NULL;
	int i=1;
	sprintf(command, "pdfinfo %s", filename);
	pdfinfo = popen("date", "r");
	printf("pdfinfo open\n");
	if(pdfinfo == NULL){
		log_msg("Could not open pipe for output");
		return KW_ERROR;
	}
	printf("pdfinfo ready\n");
	while(fgets(buffer, 512 , pdfinfo) != NULL) {
		printf("pdfinfo %s \n", buffer);
		log_msg("%2d:%s",i++,buffer); 
	}
	
	if (pclose(pdfinfo) != 0) {
		log_msg(" Error: Failed to close command stream");
	}
	printf("pdfinfo closed\n");
	return KW_SUCCESS;
}

static bool is_of_type(const char *filepath)
{
	char buffer[512];
	pipepdfinfo(filepath, buffer);
	return false;
	return true; /*extract returns data */
}

static int metadata_extract(const char *filename, struct kw_metadata *s)
{
	char *memchar = NULL;
	char buffer[1024];
	
	if (!is_of_type(filename)) {
		return KW_ERROR;
	}
	
	s->obj = NULL;
	s->do_cleanup = &do_on_cleanup;
	
	s->type = strdup("Audio");
	s->tagc = 4;
	s->tagtype = (char **)malloc(4 * sizeof(char *));
	s->tagv = (char **)malloc(4 * sizeof(char *));
	
	memchar = strdup("title");
	s->tagtype[0] = memchar;
	memchar = strdup("artist");
	s->tagtype[1] = memchar;
	memchar = strdup("album");
	s->tagtype[2] = memchar;
	memchar = strdup("genre");
	s->tagtype[3] = memchar;
	
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
	
	s->obj = NULL;
	s->do_init = &do_on_init;
	s->do_cleanup = &do_on_cleanup;
	
	return KW_SUCCESS;
}

static int metadata_update(const char *filename, struct kw_metadata *s)
{
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

struct plugin_extraction_entry *load_extractor_plugin()
{
	static struct plugin_extraction_entry *plugin = NULL;
	if (plugin == NULL) {
		plugin = (struct plugin_extraction_entry *) 
		         malloc(sizeof(struct plugin_extraction_entry));
		plugin->name = strdup("extractor");
		plugin->type = strdup("Multi");
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
