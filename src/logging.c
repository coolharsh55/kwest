/**
 * @file logging.c
 * @brief functions for logging filesystem operations
 * @author Harshvardhan Pandit
 * @date December 2012
 */

/* LICENSE
 * Copyright 2013 Harshvardhan Pandit
 * Licensed under the Apache License, Version 2.0 (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * 	http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, 
 * software distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <pwd.h>
#include <unistd.h>
#include <stdarg.h>

#include "logging.h"
#include "flags.h"


/**
 * @brief Get logfile
 * @param void
 * @return logfile : SUCCESS, NULL : FAIL
 * @author HP
 */
static void *get_logfile(void)
{
	static FILE *logfile = NULL; /* file pointer for logfile */
	struct passwd *pw = NULL;
	const char *homedir = NULL;
	char *kwestdir = NULL;
	
	if(logfile == NULL) {
		/* Set path for database file to /home/user/.config */
		pw = getpwuid(getuid());
		homedir = pw->pw_dir; /* initial working directory */
		kwestdir = (char *)malloc(QUERY_SIZE * sizeof(char));
		strcpy(kwestdir,strcat((char *)homedir,CONFIG_LOCATION));
		
		if(mkdir(kwestdir, KW_STDIR) == -1 && errno != EEXIST) {
			return NULL;
		} 
		strcat(kwestdir, LOGFILE_STORAGE);
		logfile = fopen(kwestdir,"w");
		free(kwestdir);
	}
	return logfile;
}

/**
 * @brief initialize logging
 * @param void
 * @return KW_SUCCESS : SUCCESS, KW_FAIL : FAIL
 * @author HP
 */
int log_init(void)
{
	if(get_logfile() == NULL) {
		return KW_FAIL;
	}
	return KW_SUCCESS;
}

/**
 * @brief print log message to logfile
 * @param msg - Message to be displayed in logfile
 * @return void
 * @author HP 
 */
void log_msg(const char *msg, ...)
{
    va_list argptr;
    va_start(argptr, msg);
    vfprintf(get_logfile(), msg, argptr);
    vfprintf(get_logfile(), "\n", argptr);
    va_end(argptr);
}

/**
 * @brief close the logfile
 * @param void
 * @return KW_SUCCESS : SUCCESS, -errorno : ERROR
 * @author HP
 */
int log_close(void)
{
	if(fclose(get_logfile()) == 0) {
		return KW_SUCCESS;
	}
	return -errno;
}
