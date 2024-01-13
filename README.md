# httpd
HTTP Server Written in C for (embedded) Linux systems

# Description
This is a http server written in C/C++ Programming language, that support multithreads, session management using SQLITE3 db and dynamic pages using C/C++ plugins.   
This application is meant to be used for embedded systems but there is no limitations on its usage as of now.   
It was written in 2007 by me (Anoop Kumar Narayanan). I am right now cleaning up the code.   
Removing some features and adding new features.   

Authors:
--------
Anoop Kumar Narayanan   
Bangalore   
India   
anoop.kumar.narayanan@gmail.com   


LICENSE
-------
GPLv2/GPLv3/MIT/BSD 3-clause


For:
PROPRIETARY LICENSE
Contact through mail for purchase


Features:
---------
1. Supports HTTP 1.0, works with most modern browsers
2. Supports Inbuilt Session Management for login pages
3. Supports Multilevel administration pages ( super > root > admin )
4. Supports Inbuilt sqlite3 database for use with plugins, with ability to disable plugins
5. Supports Plugins based dynamic pages, write dynamic pages in c code
6. Supports Multithreads for plugin execution, thread pool for dynamic page handling (configurable)
7. Supports Chunked data transfer for dynamic pages.
8. Supports GET and POST queries
9. Supports SSL through redirect lighttpd <---- redirect ---> httpd (>= 0.24.1) as of now. (Should SSL reinventing the wheel happen ?)
10. Supports Large file upload through POST multipart (>= 5.0GB)
11. Supports CppThreads (-DUSE_CPP11THREAD) and Pthreads ( -DUSE_PTHREAD)


COMPILE:
--------
make clean   
make


RUN:
----
./httpdsrv.sh    


NOTE:
-----
Run from source folder.   
Pages/ folder is where the HTTP webpages are located.   
sthtml are static html documents that can be served as common html documents between users.    
html documents have a suffix "username_", these documents gets served only to appropriate users.   
images like jpg, bmp and png are common files.   
Pages/router/ folder contains sample embedded router html documents without backend interface.   
login.html and public.html are the only two pages that return as themselves.



PAGE MAPPING:
--------------
when logged in as root
root.html -> root_root.html
image.jpg -> image.jpg
index.sthtml -> index.sthtml
login.xyz    -> plugin liblogin.so dynamic page
p3.xyz       -> plugin libp3.so dynamic page

When logged in as super
root.html -> super_root.html
image.jpg -> image.jpg
index.sthtml -> index.sthtml
login.xyz    -> plugin liblogin.so dynamic page
p3.xyz       -> plugin libp3.so dynamic page


Test and Execution:
-------------------
read TEST.txt

Dependency:
-----------
Read DEPENDENCY file.



