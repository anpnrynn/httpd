# httpd
HTTP Server Written in C for (embedded) Linux systems


# Release naming convention
httpd - (MAJOR_NUMBER).(YY).(MM)-(RELEASE_NUMBER_IN_MONTH)   
Use the latest release zip files, they are stable. The main branch is not stable code.


# Description
This is a http server written in C/C++ Programming language, that support threads, session management using SQLITE3 db and dynamic pages using C/C++ plugins.   
The main thread is used to assign tasks to the threadpool, throttle, send and receive static data while the threadpool is used to handle dynamic pages.   
This application is meant to be used for embedded systems but there aren't any limitations on its usage as of now.  
Supports HTTP HTTPS over IPv4 and IPv6.


# History 
It was written in 2007 by me. Currently cleaning up the code. Removing some features and adding new features.   


# Memory Consumption (initial)
0    threads = Only static file access = 30 MB    
10   threads = 140MB   
100  threads = 750MB   
300  threads = 2.3GB    
Memory grows as the session information increases    


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
1. Supports HTTP 1.1, works with most modern browsers
2. Supports Inbuilt Session Management for login pages
3. Supports Multilevel administration pages ( super > root > admin )
4. Supports Inbuilt sqlite3 database for use with plugins, with ability to disable plugins
5. Supports Plugins based dynamic pages, write dynamic pages in c code
6. Supports Multithreads for plugin execution, thread pool for dynamic page handling (configurable)
7. Supports Chunked data transfer for dynamic pages.
8. Supports GET and POST queries.
9. Supports Large file upload through POST multipart (>= 5.0GB)
10. Supports CppThreads (-DUSE_CPP11THREAD) and Pthreads ( -DUSE_PTHREAD)
11. Supports thttpd like operation by setting threads to 0 during startup. Becomes a static http file server.
12. Supports trivial DOS prevention.    
13. Supports both IPv4 and IPv6 sockets, http over IPv4 and IPv6 is now possible.    
14. Supports native SSL sockets.
15. Supports multipart.
16. Supports both linux and windows (use makefile.win under msys2 environment).


COMPILE:
--------
make distclean   
make    
make install    
cd /tmp/httpd/bin/ 

COMPILE (Under Windows + msys2 )
--------------------------------
make -f makefile.win clean   
make -f makefile.win   
make -f makefile.win install   
   
RUN:
----
./httpdsrv.sh    
log file httpd.log is generated in the same folder, and by default loglevel is set to 1 which is the lowest.    

httpdsrv arguments
------------------
./httpdsrv <ipv4/ipv6 port> <number of threads> <dosthreshold> <ssl ipv4&ipv6 port> <debuglevel> <ipv6 address to bind to>


NOTE (VERY IMPORTANT, PLEASE READ):
-----------------------------------
1. Do not run from source folder, make install places code and webpages in /tmp/httpd/ and /tmp/var/ respectively.   
2. Pages/ folder is where the HTTP webpages are located.   
3. The sthtml pages are static html documents that can be served as common html documents to all users and do not require authentication.   
4. The html documents must have a suffix "username_", these documents requires auth and gets served only to appropriate users.   
5. Images like jpg, bmp and png are common files.   
6. Pages/router/ folder contains sample embedded router html documents without backend handlers.   
7. The pages login.html and public.html are the only two pages that do not require higher level authentication and can be accessed by the public.   
8. Plugins that comes with the product are only sample code and do not provide the security features needed by corporate and enterprise products.   
9. Scripts in the test folder are meant for stability testing and not performance testing.   
10. Downloads of static files are done on the main thread using poll (works like thttpd which i think uses select).    
11. Uploads using multipart form data are handled by the main thread using poll again and the content gets writing to a /tmp/post_%d.tmp, which remains open until deleted by the plugin developer.   


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


Default Username and Password:
------------------------------
admin / adminpassword   
root  / rootpassword   
super / superpassword   
Please change them on using this product.   


Test and Execution:
-------------------
Have a look at tests folder   
Read TEST.txt   

Dependency:
-----------
Read DEPENDENCY file.



