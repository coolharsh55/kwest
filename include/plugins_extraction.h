#ifndef PLUGINS_EXTRACTION_H
#define PLUGINS_EXTRACTION_H

#include <stdbool.h>
#include "metadata_format.h"

typedef struct plugin_extraction_entry *(*loadplugin)(void);

struct p_linkedlist
{
	struct plugin_extraction_entry *plugin;
	struct p_linkedlist *next;
};

struct plugin_extraction_entry
{
	char *name;
	char *type;
	void *obj;
	bool (*is_of_type)(const char *);
	int (*p_metadata_extract)(const char *, struct kw_metadata *);
	int (*p_metadata_update)(const char *, struct kw_metadata *);
	int (*on_load)(struct plugin_extraction_entry *);
	int (*on_unload)(struct plugin_extraction_entry *);
};

struct p_linkedlist *plugins_list(void);

int plugins_add_plugin(struct plugin_extraction_entry *);

int plugins_remove_plugin(struct plugin_extraction_entry *);

int plugins_load(struct plugin_extraction_entry *);

int plugins_load_all(void);

int plugins_unload(struct plugin_extraction_entry *);

int plugins_unload_all(void);

#endif
