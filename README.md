# httpd
HTTP Server Written in C for (embedded) Linux systems

# Release naming convention
httpd - (MAJOR_NUMBER).(YY).(MM)-(RELEASE_NUMBER_IN_MONTH)    

# Description
This is a http server written in C/C++ Programming language, that support multithreads, session management using SQLITE3 db and dynamic pages using C/C++ plugins.   
This application is meant to be used for embedded systems but there is no limitations on its usage as of now.   
It was written in 2007 by me. I am right now cleaning up the code. Removing some features and adding new features.   

Authors:
--------
Anoop Kumar Narayanan   
Bangalore   
India   
anoop.kumar.narayanan@gmail.com   


LICENSE
-------
GPLv2/GPLv3


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


NOTE (VERY IMPORTANT, PLEASE READ):
-----------------------------------
1. Run from source folder.   
2. Pages/ folder is where the HTTP webpages are located.   
3. sthtml are static html documents that can be served as common html documents between users.    
4. html documents have a suffix "username_", these documents gets served only to appropriate users.   
5. images like jpg, bmp and png are common files.   
6. Pages/router/ folder contains sample embedded router html documents without backend interface.   
7. login.html and public.html are the only two pages that return as themselves.
8. plugins that comes with the product are only sample code and do not provide the security features need by corporate and enterprise products.


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



