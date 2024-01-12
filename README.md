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
GPLv2/GPLv3/BSD/MIT

Features:
---------
1. Supports HTTP 1.0, works with most modern browsers
2. Supports Inbuilt Session Management for login pages
3. Supports Multilevel administration pages ( root , admin, normal user )
4. Supports Inbuilt sqlite3 database for use with plugins
5. Supports Plugins based dynamic pages, write dynamic pages in c code
6. Supports Multithreads for plugin execution, thread pool for dynamic page handling (configurable)
7. Supports Chunked data transfer for dynamic pages.
8. Supports GET and POST queries


Dependency:
-----------
Read DEPENDENCY file.



