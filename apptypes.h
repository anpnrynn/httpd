//Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3
#ifndef  TYPES_HTTPD_H
#define  TYPES_HTTPD_H

#include <iostream>
#include <vector>
#ifndef LINUX_BUILD
#include <map>
#else
#include <unordered_map>
#include <atomic>
#endif
#include <string>

using namespace std;

//#define  MAXBUF    16384
#define  MAXBUF    65536
#define  GMT_INDIA 19800
//#define  SMALLBUF  4096
#define  SMALLBUF  65536

class ACL {
    public:
        bool invert;
        bool ipv4;
        bool subnetmask;
        bool prefixmask;
        string ip;
        long long int epochtime;
        atomic_int counter;
        bool block;
};

#ifdef LINUX_BUILD
typedef unordered_map<string, int>    MapActivePlugins;
typedef unordered_map<string, ACL*>   MapACL;
typedef unordered_map<int, string>    MapIntStr;
typedef unordered_map<int, string>    HttpCodes;
typedef unordered_map<string, string> MapStrStr;
typedef unordered_map<string, string> COOKIES;
typedef unordered_map<string, void*>  MapHttpHdlr;
typedef vector<string>     Vector;
#else
typedef map<string, int>    MapActivePlugins;
typedef map<string, ACL*>   MapACL;
typedef map<int, string>    MapIntStr;
typedef map<int, string>    HttpCodes;
typedef map<string, string> MapStrStr;
typedef map<string, string> COOKIES;
typedef map<string, void*>  MapHttpHdlr;
typedef vector<string>     Vector;
#endif

#endif
