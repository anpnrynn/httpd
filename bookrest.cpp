//Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3
#include <httpconn.h>
#include <plugin.h>
#include <httphandlers.h>
#include <sstream>
#include <multipart.h>
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
int bookrest_processReq ( Connection *conn );

struct PluginHandler pInfo = { 0x00, 0x01, 1, "bookrest.xyz", "", NULL, plugin_init, bookrest_processReq, plugin_exit };

int plugin_init() {
    HttpHandler *hHdlr = HttpHandler::createInstance();

    if ( hHdlr )
    { hHdlr->addHandler ( pInfo.sname, ( void * ) &pInfo ); }

    debuglog(" PLUGIN: %s loaded \n", pInfo.sname );
    return 0;
}

int plugin_exit() {
    return 0;
}

int bookrest_processReq ( Connection *conn ) {
    if ( !conn )
    { return 1; }


    //Read remaining post data if there are any
    debuglog (  "DBUG: Reading remaining post data %ld <-> %ld \n",
                conn->req.cLen, conn->req.rLen );

    if ( conn->req.cLen > conn->req.rLen )
    { conn->req.processHttpPostData ( conn ); }

    debuglog (  "DBUG: Read remaining post data %ld <-> %ld \n",
                conn->req.cLen, conn->req.rLen );

    if( conn->req.isMultipart() ){
	//int fd = open(argv[1], O_LARGEFILE );
	debuglog( "ERRR: multipart data handling being done, with boundary : %s \n", conn->req.getBoundary() );
        MultipartReader *mpr = new MultipartReader ( conn->req.postfd, conn->req.getBoundary() );
	mpr->processMultipartData( 1048576 );
    }
    
    conn->req.removePostTempFile();

    string output = "200 OK File uploaded \n";
    int size = output.length();

    conn->resp.setContentLen ( size );
    conn->resp.setContentType ( "text/plain" );
    sendConnRespHdr ( conn, HTTP_RESP_OK );

    if ( 0 != sendConnectionData ( conn, ( unsigned char * )output.c_str(), size ) ) {
        debuglog (  "ERRR: Unable to send dynamic data of size = %d \n", size );
    } else {
        debuglog (  "INFO: Sent dynamic data of size = %d \n", size );
    }

	//postprocessing 


    return 0;
}

