#include "flags.h"
#include "metadata_format.h"
#include "logging.h"
#include "plugins_extraction.h"

#include <string.h>

/*
static int dump_metadata(struct kw_metadata *s) 
{
	if (s->do_init != NULL) {
		s->do_init(NULL);
	}
	printf("-------------------------------------------------\n");
	printf("type:%s\n", s->type);
	printf("tagc:%d\n", s->tagc);
	for (int ret = 0 ; ret <s->tagc ; ret++) {
		printf("tagtype%d:%s\n",ret, s->tagtype[ret]);
		printf("tagv%d:%s\n",ret, s->tagv[ret]);
	}
	if (s->obj != NULL) {
		printf("metadata has embedded object\n");
	}
	printf("-------------------------------------------------\n");
	return KW_SUCCESS;
}
*/

int metadata_extract(const char *file, struct kw_metadata *s)
{
	s = memset((void *)s, 0, sizeof(struct kw_metadata));
	
	struct p_linkedlist *head = plugins_list();
	while (head != NULL) {
		if (head->plugin->is_of_type(file) == true) {
			int ret = head->plugin->p_metadata_extract(file, s);
			if (ret == KW_SUCCESS) {
				/*dump_metadata(s);*/
				return KW_SUCCESS;
			} else {
				return KW_FAIL;
			}
			break;
		} else {
		}
		head = head->next;
	}
	return KW_FAIL;
}

int metadata_update(const char *file, struct kw_metadata *s)
{
	/*s = memset((void *)s, 0, sizeof(struct kw_metadata));*/
	struct p_linkedlist *head = plugins_list();
	while (head != NULL) {
		if (head->plugin->is_of_type(file) == true) {
			int ret = head->plugin->p_metadata_update(file, s);
			if (ret == KW_SUCCESS) {
				/*dump_metadata(s);*/
				return KW_SUCCESS;
			} else {
				return KW_FAIL;
			}
			break;
		} else {
			
		}
		head = head->next;
	}
	return KW_FAIL;
}
