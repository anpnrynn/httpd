//Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3
#include <iostream>
#include <thread>
using namespace std;

#include <httpconn.h>
#include <plugin.h>
#include <httphandlers.h>
#include <sstream>
using namespace std;

/*
struct PluginHandler{
    unsigned char type;
    unsigned char authLvlReq;
    unsigned int  version;
    char sname[40];
    char aname[40];
    void (*init)();
    void (*processReq)(Connection *);
    void (*exit)();
};
*/

int plugin_init();
int plugin_exit();
int p3_processReq ( Connection *conn );

struct PluginHandler pInfo = { 0x00, 0x01, 1, "p3.xyz", "", NULL, plugin_init, p3_processReq, plugin_exit };

int plugin_init() {
    HttpHandler *hHdlr = HttpHandler::createInstance();

    //printf("Cool : plugin is getting inited\n");
    if ( hHdlr )
    { hHdlr->addHandler ( pInfo.sname, ( void * ) &pInfo ); }
    debuglog(" PLUGIN: %s loaded \n", pInfo.sname );

    return 0;
}

int plugin_exit() {
    return 0;
}

int p3_processReq ( Connection *conn ) {
    if ( !conn )
    { return 1; }


    //Read remaining post data if there are any
    debuglog (  "DBUG: Reading remaining post data %ld <-> %ld \n",
                conn->req.cLen, conn->req.rLen );

    if ( conn->req.cLen > conn->req.rLen )
    { conn->req.processHttpPostData ( conn ); }

    debuglog (  "DBUG: Read remaining post data %ld <-> %ld \n",
                conn->req.cLen, conn->req.rLen );

    //int     rc = 0;
    conn->req.removePostTempFile();

    stringstream *output = new stringstream();

    *output << "<xml><data>";


    string xmldata = "<plugininfo>"
                     "This is a plugin output and nothing else."
                     "</plugininfo>";
    *output << xmldata;
    *output << "</data></xml>";

    unsigned int size = output->str().size();
    conn->resp.setContentLen ( size );
    conn->resp.setContentType ( "text/xml" );
    sendConnRespHdr ( conn, HTTP_RESP_OK );

    if ( 0 != sendConnectionData ( conn, ( unsigned char * ) output->str().c_str(), size ) ) {
        debuglog (  "ERRR: Unable to send dynamic data of size = %d \n", size );
    } else {
        debuglog (  "INFO: Sent dynamic data of size = %d \n", size );
    }

    if ( output )
    { delete output; }

    return 0;
}

