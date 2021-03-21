#ifndef HTTP_HANDLERS_H
#define HTTP_HANDLERS_H

#include <iostream>
#include <string>
#include <apptypes.h>
#include <httpconn.h>
//#include <prlink.h>
#include <prwrapper.h>
using namespace std;

#define  HANDLER_TYPE_FILE   0
#define  HANDLER_TYPE_PLUGIN 1

#define  PERM_BILLER     0
#define  PERM_MANAGER    1
#define  PERM_ADMIN      2
#define  PERM_ROOT       3

struct PluginHandler {
    unsigned char type;
    unsigned char authLvlReq;
    unsigned int  version;
    char sname[40];
    char aname[40];
    PRLibrary    *lib;
    int ( *init ) ();
    int ( *processReq ) ( Connection * );
    int ( *exit ) ();
};

class HttpHandler {
    private:

        MapHttpHdlr *httpHdlrs;
        HttpHandler();
        static HttpHandler *hdlrObj;

    public:
        ~HttpHandler();

        static HttpHandler* createInstance();
        void*  getHandler ( string );
        void   addHandler ( string, void *data );
        void   delHandler ( string );
};

#endif
