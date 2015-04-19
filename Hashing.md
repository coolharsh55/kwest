## Purpose ##
We use Hashing to map variable size strings (_tagname_ and _filename_) to integer values which will speed up the comparison and retrieval process in a database.

## Functions ##

  * Generate hash\_key for given file
```
int sethashfile(char *abspath)	
```
> input: absolute path of file <br>
<blockquote>return: f_hashkey <br>
note: <i>abspath</i> (Absolute Path) is the path of the file on base file         system on which fuse is mounted.</blockquote>

<ul><li>Generate hash_key for a tag<br>
<pre><code>int sethashtag(char *tagname)	<br>
</code></pre>
</li></ul><blockquote>input : tag name string <br>
return: t_hashkey <br>
note  : hashkey for a new tag to be created in kwest</blockquote>

<ul><li>Hash_key for file in kwest<br>
<pre><code>int gethashfilekey(char *path)	<br>
</code></pre>
</li></ul><blockquote>input : path of file in kwest <br>
return: f_hashkey <br>
note  : <i>path</i> can be filename or a hierarchy of tags it is associated with in kwest (For example Music/Myplaylist/Rock/numb.mp3)</blockquote>

<ul><li>Hash_key for tag in kwest<br>
<pre><code>int gethashtagkey(char *tagname)<br>
</code></pre>
</li></ul><blockquote>input : tag name string <br>
return: t_hashkey <br>
note  : Returns t_hashkey for a tag in Kwest</blockquote>

<ul><li>Retrieve filename by its hash_key<br>
<pre><code>char *gethashfile(int f_hashkey)<br>
</code></pre>
</li></ul><blockquote>input : hash_key of file <br>
return: filename string <br>
note  : Returns filename(f_name) for a file in Kwest</blockquote>


<ul><li>Retrieve tag name by its hash_key<br>
<pre><code>char *gethashtag(int t_hashkey)<br>
</code></pre>
</li></ul><blockquote>input : hash_key of tag <br>
return: tag name string <br>
note  : Returns tagname for a tag in Kwest