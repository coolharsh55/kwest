#include "plugins_extraction.h"
#include "flags.h"

#include <stdio.h>
#include <stdlib.h>

#define SLL struct p_linkedlist

static SLL *plugins_get_list()
{
	static SLL listhead = {NULL, NULL};

	return (&listhead);
}

SLL *plugins_list()
{
	SLL *head = plugins_get_list();
	return head->next;
}

static int SLL_insert_node_with_object(void *obj)
{
	SLL *listhead = plugins_get_list();
	
	SLL *newnode = malloc(sizeof(SLL));
	newnode->plugin = obj;
	newnode->next = listhead->next;
	listhead->next = newnode;
	
	return KW_SUCCESS;
}

static int SLL_remove_node_with_object(void *p)
{
	SLL *listhead = plugins_get_list();
	SLL *t = NULL;
	
	while (listhead->next != NULL) {
		if (listhead->next->plugin == p) {
			t = listhead->next;
			listhead->next = t->next;
			free(t->plugin);
			free(t);
			break;
		} else {
			listhead = listhead->next;
		}
	}
	
	return KW_SUCCESS;
}

int plugins_add_plugin(struct plugin_extraction_entry *p)
{
	int ret = SLL_insert_node_with_object(p);
	
	return ret;
}

int plugins_remove_plugin(struct plugin_extraction_entry *p)
{
	int ret = SLL_remove_node_with_object(p);
	
	return ret;
}

int plugins_load(struct plugin_extraction_entry *p)
{
	return p->on_load(p);
}

int plugins_unload(struct plugin_extraction_entry *p)
{
	return p->on_unload(p);
}

int plugins_load_all()
{
	SLL *listhead = plugins_get_list();
	while (listhead->next != NULL) {
		listhead = listhead->next;
		listhead->plugin->on_load(listhead->plugin);
		listhead = listhead ->next;
	}
	
	return KW_SUCCESS;
}
