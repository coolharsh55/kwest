#Short tutorial on setup of our system

# Required installations #

## FUSE ##
### Installation ###
Download fuse from [Sourceforge](http://sourceforge.net/projects/fuse/files/fuse-2.X/)

OR

```
sudo apt-get libfuse-dev
sudo apt-get fuse-utils
```

### Compiling fuse C programs ###
```
gcc -Wall -o <output file> <input file> -D_FILE_OFFSET_BITS=64 -lfuse
```

## SQLite ##
### Installation ###

```
sudo apt-get install sqlite3  
sudo apt-get install libsqlite3-dev  
```

## GCC ##