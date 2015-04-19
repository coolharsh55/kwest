#Installing Taglib

# Installation #
[Taglib at Github Site](http://taglib.github.com/) <br>
<a href='https://github.com/taglib/taglib/archive/stable.zip'>Download Taglib-stable</a>

<b>depends on</b> <br>
TagLib requires g++ (CXX) compiler to compile<br>
<pre><code>sudo apt-get install g++<br>
</code></pre>

Download & extract<br>
Then -<br>
<pre><code>cmake -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_RELEASE_TYPE=Release -DWITH_MP4=ON -DWITH_ASF=ON<br>
make<br>
sudo make install<br>
</code></pre>

compile your programs with<br>
<pre><code>gcc -o test test.c -ltag_c<br>
gcc -o test test.cpp $(pkg-config --libs --cflags taglib)<br>
</code></pre>

For TagLib C API refer<br>
<pre><code>/usr/include/taglib/tag_c.h<br>
</code></pre>