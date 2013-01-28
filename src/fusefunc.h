/* fusefunc.h
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
 
 #ifndef KWEST_FUSEFUNC_H
 #define KWEST_FUSEFUNC_H


/* call_fuse_daemon
 * pass control to fuse daemon
 *  
 * @param int argc
 * @param int argv
 * @return 0: SUCCESS, -errno: error
 * @author @HP
 * 
 */
int call_fuse_daemon(int argc, char **argv);


#endif
