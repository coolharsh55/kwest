#include "flags.h"
#include "metadata_format.h"
#include "logging.h"
#include "plugins_extraction.h"

#include <string.h>


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
		printf("tagv%d:%s\n",ret, s->tagv[ret]==NULL?"NULL":s->tagv[ret]);
	}
	if (s->obj != NULL) {
		printf("metadata has embedded object\n");
	}
	printf("-------------------------------------------------\n");
	return KW_SUCCESS;
}

static int plugin_extractor(const char *filepath) {
	FILE *extractinfo = NULL;
	char buffer[512];
	char command[20];
	void *rval = NULL;
	
	/* code type 1 : get all possible info extract has to offer */
	/*sprintf(command, "extract %s", filepath);
	extractinfo = popen(command, "r");
	
	if (extractinfo == NULL){
		return KW_FAIL;
	}

	rval = fgets(buffer, 512 , extractinfo);
	if (rval != NULL) {
		rval = fgets(buffer, 512 , extractinfo);
	}
	if (rval == NULL) {
		printf("file mimetype not recognized by extract tool\n");
		return KW_FAIL;
	}
	
	while (rval != 0) {
		rval = fgets(buffer, 512 , extractinfo);
		printf("%s",buffer); 
	} */
	
	/* code type 2 : get mimetype only */
	sprintf(command, "extract -p mimetype %s", filepath);
	extractinfo = popen(command, "r");
	
	if (extractinfo == NULL){
		return KW_FAIL;
	}

	rval = fgets(buffer, 512 , extractinfo);
	if (rval != NULL) {
		rval = fgets(buffer, 512 , extractinfo);
	}
	if (rval == NULL) {
		printf("file mimetype not recognized by extract tool\n");
		return KW_FAIL;
	}
	rval = fgets(buffer, 512 , extractinfo);
	printf("%s",buffer); 

	if (pclose(extractinfo) != 0) {
		return KW_FAIL;
	}
	
	return KW_SUCCESS;
}

static int plugin_poppler(const char *filepath) {
	FILE *pdfinfo = NULL;
	char buffer[512];
	char command[20];
	int i = 0;
	
	sprintf(command, "pdfinfo %s", filepath);
	printf("%s\n", command);
	
	pdfinfo = popen(command, "r");
	
	if (pdfinfo == NULL){
		return KW_FAIL;
	}
	
	while (fgets(buffer, 512 , pdfinfo)) {
		printf("%s",buffer); 
	}
	if (i == 0) {
		printf("file mime is not PDF\n");
		return KW_FAIL;
	}
	
	if (pclose(pdfinfo) != 0) {
		fprintf(stdout," Error: Failed to close command stream \n");
		return KW_FAIL;
	}
	
	return KW_SUCCESS;
}

int metadata_extract(const char *file, struct kw_metadata *s)
{
	struct p_linkedlist *head = plugins_list();
	s = memset((void *)s, 0, sizeof(struct kw_metadata));
	
	while (head != NULL) {
		if (head->plugin->is_of_type(file) == true) {
			printf("plugin used: %s\n",head->plugin->name);
			int ret = head->plugin->p_metadata_extract(file, s);
			if (ret == KW_SUCCESS) {
				/*dump_metadata(s);*/
				return KW_SUCCESS;
			} else {
				//return KW_FAIL;
			}
		} else {
		/*	if (plugin_poppler(file)!= KW_SUCCESS) {
				if (plugin_extractor(file) != KW_SUCCESS) {
					return KW_FAIL;
				}
			}
			*/
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
		} 
		head = head->next;
	}
	return KW_FAIL;
}
