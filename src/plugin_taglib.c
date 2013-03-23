#include "plugin_taglib.h"
#include "plugins_extraction.h"
#include "magicstrings.h"
#include "flags.h"
#include "dbbasic.h"
#include "dbplugin.h"
#include "kw_taglib_update_metadata.hpp"
#include "logging.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <taglib/tag_c.h>

static bool is_of_type(const char *filepath)
{
	TagLib_File* file = NULL;
	file = taglib_file_new(filepath);
	if (file == NULL) {
		/* log_error("is_audio","ERROR opening file"); */
		return false;
	}
	if (taglib_file_is_valid(file) != 0) {
		taglib_file_free(file);
		return true;
	} else {
		taglib_file_free(file);
		return false;
	}
	return true;
}

static int do_on_init(void *obj)
{
	(void)obj;
	return KW_SUCCESS;
}

static int initializations(void)
{
	add_mime_type(TAG_AUDIO);
	add_metadata_type(TAG_AUDIO, TAG_ARTIST);
	add_metadata_type(TAG_AUDIO, TAG_ALBUM);
	add_metadata_type(TAG_AUDIO, TAG_GENRE);

	log_msg("plugin taglib has added database entries\n");
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
		associate_file_metadata(TAG_ALBUM,s->tagv[2],s->obj);
		associate_tag_metadata(TAG_ARTIST,s->tagv[1],
		                        TAG_ALBUM,s->tagv[2]);
		associate_tag_metadata(TAG_GENRE,s->tagv[3],
		                        TAG_ARTIST,s->tagv[1]);

	}

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

	free(s->tagtype); free(s->tagv);
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

static int metadata_extract(const char *filename, struct kw_metadata *s)
{
	char *memchar = NULL;
	s->obj = NULL;
	s->do_cleanup = &do_on_cleanup;
	if (!is_of_type(filename)) {
		return KW_ERROR;
	}

	TagLib_File* file = taglib_file_new(filename);
	TagLib_Tag* tag = taglib_file_tag(file);

	s->type = strdup(TAG_AUDIO);
	s->tagc = 4;
	s->tagtype = (char **)malloc(4 * sizeof(char *));
	s->tagv = (char **)malloc(4 * sizeof(char *));

	memchar = strdup(TAG_TITLE);
	s->tagtype[0] = memchar;
	memchar = strdup(TAG_ARTIST);
	s->tagtype[1] = memchar;
	memchar = strdup(TAG_ALBUM);
	s->tagtype[2] = memchar;
	memchar = strdup(TAG_GENRE);
	s->tagtype[3] = memchar;

	memchar = strdup(taglib_tag_title(tag));
	memchar = format_string(memchar);
	s->tagv[0] = memchar;
	memchar = strdup(taglib_tag_artist(tag));
	memchar  = format_string(memchar);
	s->tagv[1] = memchar;
	memchar = strdup(taglib_tag_album(tag));
	memchar = format_string(memchar);
	s->tagv[2] = memchar;
//	log_msg("artist: %s", s->tagv[1]==NULL?"NULL":s->tagv[1]);
//	log_msg("album: %s", s->tagv[2]==NULL?"NULL":s->tagv[2]);
	if (strcmp(memchar,"") != 0) {
		if (strcmp(memchar, s->tagv[1]) == 0) {
			memchar = (char *)malloc(strlen(s->tagv[2])+8 * sizeof(char));
			memchar = strcpy(memchar, s->tagv[2]);
			memchar = strcat(memchar, "(Album)");
			free(s->tagv[2]);
			s->tagv[2] = memchar;
		}
	}
	memchar = strdup(taglib_tag_genre(tag));
	memchar = format_string(memchar);
	s->tagv[3] = memchar;

	s->obj = file;
	s->do_init = &do_on_init;
	s->do_cleanup = &do_on_cleanup;

	taglib_tag_free_strings();
	taglib_file_free(file);

	return KW_SUCCESS;
}

static int metadata_update(const char *filename, struct kw_metadata *s)
{
	if (!is_of_type(filename)) {
		return KW_ERROR;
	}
	taglib_cpp_update_metadata(filename, s);
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

struct plugin_extraction_entry *load_taglib_plugin()
{
	static struct plugin_extraction_entry *plugin = NULL;
	if (plugin == NULL) {
		plugin = (struct plugin_extraction_entry *)
		         malloc(sizeof(struct plugin_extraction_entry));
		plugin->name = strdup("taglib_audio");
		plugin->type = strdup(TAG_AUDIO);
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

