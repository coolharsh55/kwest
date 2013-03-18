/**
 * @file plugin_extractor.c
 * @brief extract command wrapper for files
 * @author Harshvardhan Pandit
 * @date March 2013
 */
 
#include "plugins_extraction.h"
#include "plugin_libextractor.h"
#include "flags.h"
#include "dbbasic.h"
#include "dbplugin.h"
#include "logging.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define BUFFER_LENGTH 1024

#define TAG_IMAGE "Image"
#define TAG_IMAGE_CREATOR "ImageCreator"
#define TAG_IMAGE_DATE "ImageDate"
#define TAG_VIDEO "Video"
#define TAG_VIDEO_LENGTH "VideoLength"
#define TAG_VIDEO_LENGTH_SHORT "ShortVideo"
#define TAG_VIDEO_LENGTH_AVERAGE "AverageVideo"
#define TAG_VIDEO_LENGTH_LONG "LongVideo"

static int do_on_init(void *obj)
{
	(void)obj;
	return KW_SUCCESS;
}

static int initializations(void)
{
	add_mime_type(TAG_IMAGE);
	add_metadata_type(TAG_IMAGE, TAG_IMAGE_CREATOR);
	add_metadata_type(TAG_IMAGE, TAG_IMAGE_DATE);
	add_mime_type(TAG_VIDEO);
	add_metadata_type(TAG_VIDEO, TAG_VIDEO_LENGTH);
	log_msg("plugin extractor has added database entries\n");
	return KW_SUCCESS;
}

static int associate_image_metadata(struct kw_metadata *s) {
	associate_file_metadata(TAG_IMAGE_CREATOR, s->tagv[0], s->obj);
	associate_file_metadata(TAG_IMAGE_DATE, s->tagv[1], s->obj);
	return KW_SUCCESS;
}

static int associate_video_metadata(struct kw_metadata *s) {
	associate_file_metadata(TAG_VIDEO_LENGTH, s->tagv[0], s->obj);
	return KW_SUCCESS;
}

static int do_on_cleanup(struct kw_metadata *s)
{
	int i=0;
	char *memchar = NULL;
	if (s->obj != NULL) {
		if (s->type == NULL) {
			return KW_ERROR;
		}
		if (strcmp(s->type, TAG_IMAGE) == 0) {
			associate_image_metadata(s);
		} else if (strcmp(s->type, TAG_VIDEO) == 0) {
			associate_video_metadata(s);
		}
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
	if (s->type != NULL) {
		free(s->type);
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
	
	while (*strptr != '-') {
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

static char *format_date(char *string) {
	char *Month[13];
	Month[0] = "NA";
	Month[1] = "Jan";Month[2] = "Feb";Month[3] = "Mar";Month[4] = "Apr";
	Month[5] = "May";Month[6] = "Jun";Month[7] = "Jul";Month[8] = "Aug";
	Month[9] = "Sep";Month[10] = "Oct";Month[11] = "Nov";Month[12] = "Dec";
	char *endptr;
	char *newDate = (char *)malloc(8 * sizeof(char));
	int val = 0;
	
	while (*string != '-') { string++; } string++; string++;
	val = (int)strtol(string, &endptr, 10);
	sprintf(newDate, "%d", val);
	string = ++endptr;
	val = (int)strtol(string, &endptr, 10);
	strcat(newDate, Month[val]);
	newDate[7] = '\0';
	return newDate;
}

static int pipe_extract_image(const char *filename, struct kw_metadata *s) {
	char buffer[BUFFER_LENGTH];
	char command[BUFFER_LENGTH];
	FILE *pipe_extract = NULL;
	
	s->type = strdup(TAG_IMAGE);
	s->tagc = 2;
	s->tagtype = (char **)malloc(2 * sizeof(char *));
	s->tagv = (char **)malloc(2 * sizeof(char *));
	s->tagtype[0] = strdup(TAG_IMAGE_CREATOR);
	s->tagv[0] = NULL;
	s->tagv[1] = NULL;
	s->tagtype[1] = strdup(TAG_IMAGE_DATE);
	
	sprintf(command, "extract %s", filename);
	pipe_extract = popen(command, "r");
	if(pipe_extract == NULL){
		log_msg("Could not open pipe for output");
		return KW_ERROR;
	}
	while (fgets(buffer, BUFFER_LENGTH , pipe_extract) != NULL) {
		if (strstr(buffer, "camera model - ") != NULL || 
		    strstr(buffer, "created by software - ")) {
			s->tagv[0] = strdup(format_string(buffer));
		}
		else if (strstr(buffer, "creation date - ") != NULL) {
			s->tagv[1] = format_date(buffer);
		}
	}
	return KW_SUCCESS;
}

static int pipe_extract_video(const char *filename, struct kw_metadata *s) {
	char buffer[BUFFER_LENGTH];
	char command[BUFFER_LENGTH];
	char *strptr = buffer;
	char *endptr;
	long duration = 0;
	FILE *pipe_extract = NULL;
	
	s->type = strdup(TAG_VIDEO);
	s->tagc = 1;
	s->tagtype = (char **)malloc(2 * sizeof(char *));
	s->tagv = (char **)malloc(2 * sizeof(char *));
	s->tagtype[0] = strdup(TAG_VIDEO_LENGTH);
	s->tagv[0] = NULL;
	
	sprintf(command, "extract -p duration %s", filename);
	pipe_extract = popen(command, "r");
	if(pipe_extract == NULL){
		log_msg("Could not open pipe for output");
		return KW_ERROR;
	}
	while (fgets(buffer, BUFFER_LENGTH , pipe_extract) != NULL);
	while (*strptr != '-') { strptr++; } strptr++; strptr++;
	duration = strtol(strptr, &endptr, 10);
	if (duration <= 1800) {
		s->tagv[0] = strdup(TAG_VIDEO_LENGTH_SHORT);
	} else if (duration >= 5400) {
		s->tagv[0] = strdup(TAG_VIDEO_LENGTH_LONG);
	} else {
		s->tagv[0] = strdup(TAG_VIDEO_LENGTH_AVERAGE);
	}
	return KW_SUCCESS;
}

static int pipeextract(const char *filename, struct kw_metadata *s)
{
	char buffer[BUFFER_LENGTH];
	char command[BUFFER_LENGTH];
	FILE *pipe_extract = NULL;
	sprintf(command, "extract -p mimetype %s", filename);
	pipe_extract = popen(command, "r");
	if(pipe_extract == NULL){
		log_msg("Could not open pipe for output");
		return KW_ERROR;
	}
	/* run1 determines if extract is working correctly */
	if (fgets(buffer, BUFFER_LENGTH , pipe_extract) == NULL) {
		log_msg("extract tool cannot run\n");
		return KW_ERROR;
	}
	/* run2 removes unwanted garbage line */
	if (fgets(buffer, BUFFER_LENGTH , pipe_extract) == NULL) {
		log_msg("no metadata extracted\n");
		return KW_ERROR;
	}
	/* check for runtime error */
	if (fgets(buffer, BUFFER_LENGTH , pipe_extract) == NULL) {
		log_msg("extract tool runtime error\n");
		return KW_ERROR;
	} 
	if (strstr(buffer,"mimetype - image/") != NULL) {
		if (s == NULL) {
			return KW_SUCCESS;
		}
		pipe_extract_image(filename, s);
	} else if (strstr(buffer,"mimetype - video/") != NULL) {
		if (s == NULL) {
			return KW_SUCCESS;
		}
		pipe_extract_video(filename, s);
	} else {
		log_msg("Unhandled %s\n",buffer);
		return KW_ERROR;
	}
	
	if (pclose(pipe_extract) != 0) {
		log_msg(" Error: Failed to close command stream");
	}
	return KW_SUCCESS;
}

static bool is_of_type(const char *filepath)
{
	if (pipeextract(filepath, NULL) == KW_SUCCESS) {
		return true;
	}
	return false;
}

static int metadata_extract(const char *filename, struct kw_metadata *s)
{
	pipeextract(filename, s);
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

struct plugin_extraction_entry *load_libextractor_plugin()
{
	static struct plugin_extraction_entry *plugin = NULL;
	if (plugin == NULL) {
		plugin = (struct plugin_extraction_entry *) 
		         malloc(sizeof(struct plugin_extraction_entry));
		plugin->name = strdup("extractor");
		plugin->type = strdup("Multiple");
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
