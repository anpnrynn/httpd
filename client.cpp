#include <iostream>
#include <prio.h>
#include <prnetdb.h>
#include <prthread.h>
#include <prnetdb.h>
#include <xmlparser.h>
#include <xmlmsg.h>
#include <sqlite3.h>

#define SRVPORT     1999

using namespace std;

sqlite3 *db;

int main() {
    PRFileDesc *srv[10], *client;
    PRNetAddr  srvAddr;
    PRNetAddr  clientAddr;
    char data[16384];
    char rdata[16384];
    int i;

    XmlParser *parser[10];

    strcpy ( data, "<msg><sql dir=\"0\">select * from products;</sql></msg>" );

    for ( i = 0; i < 10 ; i++ ) {

        srv[i] = PR_NewTCPSocket();
        srvAddr.inet.family = PR_AF_INET;
        srvAddr.inet.ip     = 0x00000000;
        srvAddr.inet.port   = PR_htons ( SRVPORT );

        while ( PR_Connect ( srv[i], &srvAddr, 1 ) != PR_SUCCESS ) {
            fprintf ( stderr, "Connecting \n" );
            PR_Sleep ( 1000 );
        }

        parser[i] = constructMsg();
        parser[i]->setFileDesc ( srv[i] );

        PR_Send ( srv[i], data, strlen ( data ), 0, 1 );
        //int nbytes = PR_Recv(srv[i], rdata, 16384, 0, 100);
        parser[i]->recvXmlData();
        parser[i]->parseXmlData();
        PR_Sleep ( 1000 );
    }

    PR_Shutdown ( srv[4], PR_SHUTDOWN_BOTH );
    PR_Shutdown ( srv[5], PR_SHUTDOWN_BOTH );
    PR_Shutdown ( srv[7], PR_SHUTDOWN_BOTH );
    PR_Close ( srv[4] );
    PR_Close ( srv[5] );
    PR_Close ( srv[7] );
    srv[4] = NULL;
    srv[5] = NULL;
    srv[7] = NULL;

    strcpy ( data, "< msg value=\"1\" value2=\"2\" >< sql >{col11|col12|col13}{col21|col22|col23}</sql></msg>" );

    for ( i = 0; i < 10; i++ ) {
        if ( srv[i] ) {
            PR_Send ( srv[i], data, strlen ( data ), 0, 1 );
            PR_Sleep ( 1000 );
        }
    }

    PR_Shutdown ( srv[1], PR_SHUTDOWN_BOTH );
    PR_Shutdown ( srv[2], PR_SHUTDOWN_BOTH );
    PR_Shutdown ( srv[6], PR_SHUTDOWN_BOTH );
    PR_Close ( srv[1] );
    PR_Close ( srv[2] );
    PR_Close ( srv[6] );
    srv[1] = NULL;
    srv[2] = NULL;
    srv[6] = NULL;

    for ( i = 0; i < 10; i++ ) {
        if ( srv[i] ) {
            PR_Send ( srv[i], data, strlen ( data ), 0, 1 );
            PR_Sleep ( 1000 );
            PR_Shutdown ( srv[i], PR_SHUTDOWN_BOTH );
            PR_Close ( srv[i] );
            srv[i] = NULL;
        }
    }

}
