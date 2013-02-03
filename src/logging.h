/**
 * @file logging.h
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
 
#ifndef KWEST_LOGGING_H
#define KWEST_LOGGING_H

#include <stdio.h>


/*
 * initialize logging
 */
int log_init(void);

/*
 * print log message to logfile
 */
void log_msg(const char *msg, ...);

/*
 * close the logfile
 */
int log_close(void);

#endif
