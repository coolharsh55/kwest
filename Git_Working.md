#get git on your system

# Download git #

  1. for debian/ubuntu:
```
sudo apt-get install git
```
  1. login into your account in project kwest at code.google.com
  1. goto http://code.google.com/p/kwest/source/checkout and check your password
  1. open terminal
  1. 
```
nano ~/.netrc
```
  1. copy:
```
machine code.google.com
login <userid>
password googlecode.com_password
```
  1. press _ctrl+X_ then press 'Y' and save the file
  1. 
```
nano ~/.gitconfig
```
copy and paste this in nano _(try ct+sh+V)_
```
[user]
	name = <userid>
	email = <google-account email>
[core]
	autocrlf = false
	safecrlf = true
```
  1. save this file
  1. go to the directory where you will be storing and working on the _kwest_ files
  1. type
```
git clone https://code.google.com/p/kwest/
```
  1. git will copy the repository in a folder named kwest

# Using GIT #

  * to get files from the server execute
```
git pull
```
in the directory _kwest_
  * to add files use
```
git add <filename>
```
  * to commit changes use
```
git commit -m "comment"
```
  * to push your changes to the server use
```
git push
```
  * to modify/change a file you have previously added
```
git commit -a
git push
```
this will track changes in files you have previously uploaded

## Branching ##

First make sure that you have the latest online repository copy.
If you have code you have modified and want to make a new branch from it, move it to some temporary location for the time being. Then create the branch, move the code back to this new branch, and make a commit.

Note: You won't get any new folder or structure denoting the new branch. All the folders and files will be as is. You won't even notice which branch you are in. So always check which branch you are working in.

**pre-requisite** : having same code as the online repository

  * view all current branches
```
git branch -a
```

**Local Branch**

  * create new branch on local machine
```
git branch branch_name
```

  * view all current branches on local machine
```
git branch
```

  * switch to new branch
```
git checkout branch_name
```

  * switch to main branch
```
git checkout master
```

  * merge branch\_name into master
```
git checkout master
git merge branch_name
```

  * delete branch on local machine
```
git branch -d branch_name
```

If you are in master, you will get the same code as online repository (branch master). As soon as you switch to branch\_name, all the files will be switched to that branch.

**Pushing branch on server**

These local branches will not appear online unless you push the branch on server. Thus you can maintain private branches by not pushing them.
As soon as the branch is online, others can access and commit to that branch.

Note: Commit a branch before pushing it on server.

  * push the branch on server
```
git push origin branch_name
```

  * delete branch from server
```
git push origin : branch name
```

**Pull remote branch from server**

  * create local branch and pull from server
```
git checkout -b branch_name origin/branch_name
```

# GUI Client #

**SmartGit** is an easy-to-use graphical user interface for Git and Mercurial with optimized work-flows. SmartGit supports all Git and Mercurial features needed for every-day work in software development projects: <br>
<a href='http://www.syntevo.com/smartgit/index.html'>SmarGit Home</a> <br>
<a href='http://www.syntevo.com/download/smartgit/smartgit-generic-3_0_11.tar.gz'>SmartGit Download</a>

<ol><li>download and extract to some folder<br>
</li><li>check if you have JRE installed<br>
<pre><code>java -version<br>
</code></pre>
</li><li>if not, install java<br>
<pre><code>sudo apt-get install openjdk-7-jre<br>
</code></pre>
</li><li>in the SmartGit folder, run <b>smargit.sh</b> in <i>bin</i> folder<br>
</li><li>select non-commercial use<br>
</li><li>select no hosting site<br>
</li><li>later, open the repository you have downloaded<br>
</li><li>SmartGit will recognise the git folder and you can use it as usual</li></ol>

<h1>GIT references</h1>

<a href='http://gitref.org/index.html'>GIT Reference</a> <br>
<a href='http://wiki.sourcemage.org/Git_Guide'>GIT Guide</a> <br>
<a href='http://www.spheredev.org/wiki/Git_for_the_lazy'>Git Lazy tut</a> <br>