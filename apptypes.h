#ifndef  TYPES_HTTPD_H
#define  TYPES_HTTPD_H

#include <iostream>
#include <vector>
#ifndef LINUX_BUILD
#include <map>
#else
#include <unordered_map>
#endif
#include <string>

using namespace std;

#define  MAXBUF   16384
#define  GMT_INDIA 19800
#define  SMALLBUF  4096

#ifdef LINUX_BUILD
typedef unordered_map<int, string>    MapIntStr;
typedef unordered_map<int, string>    HttpCodes;
typedef unordered_map<string, string> MapStrStr;
typedef unordered_map<string, string> COOKIES;
typedef unordered_map<string, void*>  MapHttpHdlr;
typedef vector<string>     Vector;
#else
typedef map<int, string>    MapIntStr;
typedef map<int, string>    HttpCodes;
typedef map<string, string> MapStrStr;
typedef map<string, string> COOKIES;
typedef map<string, void*>  MapHttpHdlr;
typedef vector<string>     Vector;
#endif

#endif
