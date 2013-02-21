#ifndef METADATA_FORMAT_H
#define METADATA_FORMAT_H

struct kw_metadata {
	char  *type;
	int tagc;
	char **tagtype;
	char **tagv;
	void *obj;
	int (*do_init)(void *);
	int (*do_cleanup)(struct kw_metadata *);
};

#endif
