#include "plugin_taglib.h"
#include "plugins_extraction.h"
#include "magicstrings.h"
#include "flags.h"
#include "dbbasic.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <taglib/tag_c.h>

static bool is_of_type(const char *filepath)
{
	//return false;
	printf("param:%s\n", filepath);
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
	printf("do_on_init has %s object\n",(obj!=NULL)?"a":"no");
	return KW_SUCCESS;
}

static int initializations(void)
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
	
	printf("plugin taglib has added database entries\n");
	return KW_SUCCESS;
}

static int do_on_cleanup(struct kw_metadata *s)
{
	int i=0;
	
	printf("metadata cleanup\n");
	if (s->type != NULL) {
		free(s->type);
	}
	if (s->obj != NULL) {
		associate_file_metadata(TAG_ARTIST,s->tagv[1],s->obj);
		associate_file_metadata(TAG_ALBUM,s->tagv[2],s->obj);
		associate_file_metadata(TAG_GENRE,s->tagv[3],s->obj);
	}
	for(i=0 ; i<s->tagc ; i++) {
		if (s->tagtype[i] != NULL) {
			free(s->tagtype[i]);
		}
		
		if (s->tagv[i] != NULL) {
			free(s->tagv[i]);
		}
		
	}
	return KW_SUCCESS;
}


static char *format_string(const char *origstring)
{
	int i;
	char *string = (char *)origstring;
	for (i = 0; string[i] != '\0'; i++) {
		
		if (i == 0 || !isalpha(string[i-1])) {
			string[i] = toupper(string[i]);
		} else {
			string[i] = tolower(string[i]);
		}
		if (string[i] == '/' || string[i] == '\\') {
		string[i] = ' ';
		}
	}
	
	/* Remove End of string White Spaces */
	while (string[--i]==' '); string[++i]='\0';
	
	return string;
}

static int metadata_extract(const char *filename, struct kw_metadata *s)
{
	s->obj = NULL;
	s->do_cleanup = &do_on_cleanup;
	if (!is_of_type(filename)) {		
		return KW_ERROR;
	}
	
	printf("extract metadata from file %s\n", filename);
	TagLib_File* file = taglib_file_new(filename); 
	TagLib_Tag* tag = taglib_file_tag(file);
	
	s->type = strdup("Audio");
	s->tagc = 4;
	s->tagtype = (char **)malloc(4 * sizeof(char *));
	s->tagv = (char **)malloc(4 * sizeof(char *));
	
	s->tagtype[0] = strdup("title");
	s->tagtype[1] = strdup("artist");
	s->tagtype[2] = strdup("album");
	s->tagtype[3] = strdup("genre");
	
	s->tagv[0] = strdup(format_string(taglib_tag_title(tag)));
	s->tagv[1] = strdup(format_string(taglib_tag_artist(tag)));
	s->tagv[2] = strdup(format_string(taglib_tag_album(tag)));
	s->tagv[3] = strdup(format_string(taglib_tag_genre(tag)));
	
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
	printf("updating file metdata\n");
	TagLib_File* file = taglib_file_new(filename); 
	if (file == NULL) {
		printf("Tablig file NULL error\n");
	}
	TagLib_Tag* tag = taglib_file_tag(file);
	if (tag == NULL) {
		printf("Tablig tag NULL error\n");
	}
	
	free(s->tagv[0]);
	s->tagv[0] = strdup("test title");
	taglib_tag_set_title(tag, s->tagv[0]);	
	taglib_tag_free_strings();
	taglib_file_free(file);
	
	return KW_SUCCESS;
}

static int on_load(struct plugin_extraction_entry *plugin)
{
	printf("plugin on load\n");
	printf("name: %s\n", plugin->name);
	printf("type: %s\n", plugin->type);
	printf("istype: %s\n", plugin->is_of_type("test")?"TRUE":"FALSE");
	initializations();
	return KW_SUCCESS;
}
	
static int on_unload(struct plugin_extraction_entry *plugin)
{
	printf("plugin on unload\n");
	printf("freeing name: %s\n", plugin->name);
	free(plugin->name);
	printf("freeing type: %s\n", plugin->type);
	return KW_SUCCESS;
}

struct plugin_extraction_entry *load_this_plugin()
{
	static struct plugin_extraction_entry *plugin = NULL;
	printf("PLUGIN TAGLIB\n");
	if (plugin == NULL) {
		plugin = (struct plugin_extraction_entry *) 
		         malloc(sizeof(struct plugin_extraction_entry));
		plugin->name = strdup("taglib_audio");
		printf("name : %s\n", plugin->name);
		plugin->type = strdup("Audio");
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
