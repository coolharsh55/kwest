#Functions and Data structures for used for Importing Semantics

# dirent.h #
## Data Type ##

**DIR**
A type representing a directory stream.

**structure dirent** includes the following members:

| ino\_t | d\_ino | file serial number |
|:-------|:-------|:-------------------|
| char | d\_name[.md](.md) | name of entry |

Link : http://pubs.opengroup.org/onlinepubs/009695399/basedefs/dirent.h.html

## Function ##
```
DIR *opendir(const char *);
```
open a directory

Link : http://pubs.opengroup.org/onlinepubs/007908799/xsh/opendir.html

```
int closedir(DIR *dirp);
```
close a directory stream

Link : http://pubs.opengroup.org/onlinepubs/009695399/functions/closedir.html


---


# sys/stat.h #
## Data Type ##
**structure stat** holds data returned by the stat() function

_Useful Members_

| mode\_t | st\_mode | mode of file |
|:--------|:---------|:-------------|

_Values of st\_mode_

S\_IFDIR  - directory

S\_IFREG  - regular

_Following macros test whether a file is of the specified type_

S\_ISDIR(m) - Test for a directory.

S\_ISREG(m) - Test for a regular file.

Link : http://pubs.opengroup.org/onlinepubs/7908799/xsh/sysstat.h.html

## Function ##
```
int stat(const char *path, struct stat *buf);
```
Obtains information about the named file and writes it to the area pointed to by the buf argument.
The path argument points to a pathname naming a file

Link : http://pubs.opengroup.org/onlinepubs/7908799/xsh/stat.html