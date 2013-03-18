/**
 * @file test_pdfinfo.c
 * @brief test for pdfinfo plugin
 * @author Harshvardhan Pandit
 * @date March 2013
 */


#include <stdio.h>
#include <stdlib.h>

#include "flags.h"
#include "plugins_extraction.h"
#include "plugin_pdfinfo.h"
#include "logging.h"
#include "magicstrings.h"

int test_main()
{
	printf("Test for PDFINFO plugin\n");
	log_init();
	int ret = plugins_add_plugin(load_pdfinfo_plugin());
	printf("plugin load status = %d\n", ret);
	return 0;
}
