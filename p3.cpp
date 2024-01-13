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

    return 0;
}

int plugin_exit() {
    return 0;
}

int p3_processReq ( Connection *conn ) {
    if ( !conn )
    { return 1; }


    //Read remaining post data if there are any
    fprintf( stderr, "DBUG: Reading remaining post data %ld <-> %ld \n",
		    conn->req.cLen, conn->req.rLen );
    if( conn->req.cLen > conn->req.rLen )
	    conn->req.processHttpPostData( conn );
    fprintf( stderr, "DBUG: Read remaining post data %ld <-> %ld \n",
		    conn->req.cLen, conn->req.rLen );

    //int     rc = 0;
    //conn->req.removePostTempFile();

    stringstream *output = new stringstream();

    *output << "<xml><data>";


    string xmldata = "<breakfast_menu>"
                     "<food>"
                     "<name>Belgian Waffles</name>"
                     "<price>$5.95</price>"
                     "<description>Two of our famous Belgian Waffles with plenty of real maple syrup</description>"
                     "<calories>650</calories>"
                     "</food>"
                     "<food>"
                     "<name>Strawberry Belgian Waffles</name>"
                     "<price>$7.95</price>"
                     "<description>Light Belgian waffles covered with strawberries and whipped cream</description>"
                     "<calories>900</calories>"
                     "</food>"
                     "<food>"
                     "<name>Berry-Berry Belgian Waffles</name>"
                     "<price>$8.95</price>"
                     "<description>Light Belgian waffles covered with an assortment of fresh berries and whipped cream</description>"
                     "<calories>900</calories>"
                     "</food>"
                     "<food>"
                     "<name>French Toast</name>"
                     "<price>$4.50</price>"
                     "<description>Thick slices made from our homemade sourdough bread</description>"
                     "<calories>600</calories>"
                     "</food>"
                     "<food>"
                     "<name>Homestyle Breakfast</name>"
                     "<price>$6.95</price>"
                     "<description>Two eggs, bacon or sausage, toast, and our ever-popular hash browns</description>"
                     "<calories>950</calories>"
                     "</food>"
                     "</breakfast_menu>";
    *output << xmldata;
    *output << "</data></xml>";

    unsigned int size = output->str().size();
    conn->resp.setContentLen ( size );
    conn->resp.setContentType ( "text/xml" );
    sendConnRespHdr ( conn, HTTP_RESP_OK );

    if ( 0 != sendConnectionData ( conn, ( unsigned char * ) output->str().c_str(), size ) ) {
        fprintf ( stderr, "ERRR: unable to send dynamic data of size = %d \n", size );
    } else {
        fprintf ( stderr, "INFO: able to send dynamic data of size = %d \n", size );
    }

    if ( output )
    { delete output; }

    return 0;
}

