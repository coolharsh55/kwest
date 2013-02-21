#include "plugin_taglib.h"
#include "plugins_extraction.h"
#include "magicstrings.h"
#include "flags.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

static bool is_of_type(char *filename)
{
	printf("param:%s\n", filename);
	return true;
}

static int do_on_init(void *obj)
{
	printf("do_on_init has %s object\n",(obj!=NULL)?"a":"no");
	return KW_SUCCESS;
}

static int do_on_cleanup(struct kw_metadata *s)
{
	int i=0;
	printf("metadata cleanup\n");
	if (s->type != NULL) {
		free(s->type);
	}
	for(i=0 ; i<s->tagc ; i++) {
		if (s->tagtype[i] != NULL) {
			free(s->tagtype[i]);
		}
		if (s->tagv[i] != NULL) {
			free(s->tagv[i]);
		}
	}
	if (s->obj != NULL) {
		free(s->obj);
	}
	return KW_SUCCESS;
}


static int metadata_extract(char *filename, struct kw_metadata *s)
{
	printf("extract metadata from file %s", filename);
	s->type = strdup("test");
	s->tagc = 3;
	s->tagtype = (char **)malloc(3 * sizeof(char *));
	s->tagv = (char **)malloc(3 * sizeof(char *));
	s->tagtype[0] = strdup("tagtypeA");
	s->tagtype[1] = strdup("tagtypeB");
	s->tagtype[2] = strdup("tagtypeC");
	s->tagv[0] = strdup("metaA");
	s->tagv[1] = strdup("metaB");
	s->tagv[2] = strdup("metaC");
	s->obj = NULL;
	s->do_init = &do_on_init;
	s->do_cleanup = &do_on_cleanup;
	
	return KW_SUCCESS;
}

static int metadata_update(char *filename, struct kw_metadata *s)
{
	
	return KW_SUCCESS;
}

static int on_load(struct plugin_extraction_entry *plugin)
{
	printf("plugin on load\n");
	printf("name: %s\n", plugin->name);
	printf("type: %s\n", plugin->type);
	printf("istype: %s\n", plugin->is_of_type("test")?"TRUE":"FALSE");
	
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
	if (plugin == NULL) {
		plugin = (struct plugin_extraction_entry *) 
		         malloc(sizeof(struct plugin_extraction_entry));
		plugin->name = strdup("taglib_audio");
		plugin->type = strdup("audio");
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
