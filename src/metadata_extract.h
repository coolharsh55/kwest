#ifndef METADATA_EXTRACT_H
#define METADATA_EXTRACT_H

#include "flags.h"
#include "metadata_format.h"
#include "logging.h"

#include <string.h>

int metadata_extract(const char *, struct kw_metadata *);
int metadata_update(const char *, struct kw_metadata *);

#endif
