/* import.h
 * import files into kwest
 */

/* LICENSE
 * Copyright 2013 Sahil Gupta
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

#ifndef IMPORT_H_INCLUDED
#define IMPORT_H_INCLUDED



/* import
 * Import Directory-File structure from File System to Kwest
 * 
 * @param const char *path
 * @return 1: SUCCESS, KW_FAIL: ERROR
 * @author @SG
 * 
 */
int import(const char *path);

#endif
