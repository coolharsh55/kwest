/* logging.c
 * functions for logging filesystem operations
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
#include <stdlib.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <pwd.h>
#include <string.h>
#include <unistd.h>

#include "logging.h"
#include "flags.h"

/* log_init
 * initialize logging
 * 
 * @param none
 * @return KW_SUCCESS on success, KW_FAIL on fail
 * @author @HP
 */
 
static void *get_logfile()
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
int log_init(void)
{
	if(get_logfile() == NULL) {
		return KW_FAIL;
	}
	return KW_SUCCESS;
}

/* log_msg
 * print log message to logfile
 * 
 * @param none
 * @return none
 * @author @HP 
 */
void log_msg(const char *msg, ...)
{
    va_list argptr;
    va_start(argptr, msg);
    vfprintf(get_logfile(), msg, argptr);
    vfprintf(get_logfile(), "\n", argptr);
    va_end(argptr);
}

/* log_close
 * close the logfile
 * 
 * @param none
 * @return none
 * @author @HP
 */
int log_close(void)
{
	if(fclose(get_logfile()) == 0) {
		return KW_SUCCESS;
	}
	return -errno;
}
