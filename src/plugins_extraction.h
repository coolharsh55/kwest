#ifndef PLUGINS_EXTRACTION_H
#define PLUGINS_EXTRACTION_H

#define PLUGIN_TYPE char *

#include <stdbool.h>
#include "metadata_format.h"

struct p_linkedlist
{
	struct plugin_extraction_entry *plugin;
	struct p_linkedlist *next;
};

struct plugin_extraction_entry
{
	char *name;
	PLUGIN_TYPE type;
	void *obj;
	bool (*is_of_type)(char *);
	int (*p_metadata_extract)(char *, struct kw_metadata *);
	int (*p_metadata_update)(char *, struct kw_metadata *);
	int (*on_load)(struct plugin_extraction_entry *);
	int (*on_unload)(struct plugin_extraction_entry *);
};

struct p_linkedlist *plugins_list(void);

int plugins_add_plugin(struct plugin_extraction_entry *);

int plugins_remove_plugin(struct plugin_extraction_entry *);

int plugins_load(struct plugin_extraction_entry *);

int plugins_load_all(void);

int plugins_unload(struct plugin_extraction_entry *);

#endif
