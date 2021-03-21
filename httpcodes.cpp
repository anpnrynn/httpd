#include <httpcodes.h>
#include <iostream>
#include <apptypes.h>
using namespace std;

HttpCodes *status = NULL;

void setupStatusCodes() {
    if ( !status ) {
        status = new HttpCodes();
        ( *status ) [100] = "Continue";
        ( *status ) [101] = "Switching Protocols";
        ( *status ) [102] = "Processing";
        ( *status ) [200] = "OK";
        ( *status ) [201] = "Created";
        ( *status ) [202] = "Accepted";
        ( *status ) [203] = "Non-Authoritative Information";
        ( *status ) [204] = "No Content";
        ( *status ) [205] = "Reset Content";
        ( *status ) [206] = "Partial Content";
        ( *status ) [207] = "Multi-Status";
        ( *status ) [300] = "Multiple Choices";
        ( *status ) [301] = "Moved Permanently";
        ( *status ) [302] = "Found";
        ( *status ) [303] = "See Other";
        ( *status ) [304] = "Not Modified";
        ( *status ) [305] = "Use Proxy";
        ( *status ) [307] = "Temporary Redirect";
        ( *status ) [400] = "Bad Request";
        ( *status ) [401] = "Unauthorized";
        ( *status ) [402] = "Payment Required";
        ( *status ) [403] = "Forbidden";
        ( *status ) [404] = "Not Found";
        ( *status ) [405] = "Method Not Allowed";
        ( *status ) [406] = "Not Acceptable";
        ( *status ) [407] = "Proxy Authentication Required";
        ( *status ) [408] = "Request Timeout";
        ( *status ) [409] = "Conflict";
        ( *status ) [410] = "Gone";
        ( *status ) [411] = "Length Required";
        ( *status ) [412] = "Precondition Failed";
        ( *status ) [413] = "Request Entity Too Large";
        ( *status ) [414] = "URI too long";
        ( *status ) [415] = "Unsupported Media Type";
        ( *status ) [416] = "Requested Range Not Satisfiable";
        ( *status ) [417] = "Expectation Failed";
        ( *status ) [422] = "Unprocessable Entity";
        ( *status ) [423] = "Locked";
        ( *status ) [424] = "Failed Dependency";
        ( *status ) [426] = "Upgrade Required";
        ( *status ) [500] = "Internal Server Error";
        ( *status ) [501] = "Not Implemented";
        ( *status ) [502] = "Bad Gateway";
        ( *status ) [503] = "Service Unavailable";
        ( *status ) [503] = "Gateway Timeout";
        ( *status ) [504] = "HTTP Version Not Supported";
        ( *status ) [507] = "Insufficient Storage";
    }
}

const char *getStatusInfo ( int code ) {
    static const char *empty = "";
    MapIntStr::iterator i = status->find ( code );

    if ( i != status->end() ) {
        return i->second.c_str();
    }

    return ( const char* ) empty;
}
