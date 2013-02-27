#include "kw_taglib_update_metadata.hpp"
#include "flags.h"

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <iostream>

using namespace TagLib;

int taglib_cpp_update_metadata(const char *filepath, struct kw_metadata *M)
{
	FileRef f(filepath);
	if (M->tagv[0] != NULL) {
		f.tag()->setTitle(M->tagv[0]);
	}
	if (M->tagv[1] != NULL) {
		f.tag()->setArtist(M->tagv[1]);
	}
	if (M->tagv[2] != NULL) {
		f.tag()->setAlbum(M->tagv[2]);
	}
	if (M->tagv[3] != NULL) {
		f.tag()->setGenre(M->tagv[3]);
	}
	f.save();
	
	return KW_SUCCESS;
}

