#include <httpconn.h>
#include <plugin.h>
#include <httphandlers.h>

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

int login_init();
int login_exit();
int login_processReq ( Connection *conn );

struct PluginHandler loginInfo = { 0x00, 0x01, 1, "login", "", NULL, login_init, login_processReq, login_exit };

int login_init() {
    HttpHandler *hHdlr = HttpHandler::createInstance();

    if ( hHdlr ) {
        fprintf ( stderr, "INFO: Starting Login Plugin\n" );
        hHdlr->addHandler ( loginInfo.sname, ( void * ) &loginInfo );
    }

    return 0;
}

int login_exit() {
    return 0;
}

int login_handler ( void *data, int argc, char **argv, char **col ) {
    Connection *conn = ( Connection * ) data;

    if ( conn ) {
        // add will reset the value if already present
        if ( argv[0] )
        { conn->sess->addVariable ( "auth", argv[0], 0, 0 ); }
        else
        { conn->sess->addVariable ( "auth", "1", 0, 0 ); }

        //if( argv[1] )
        //{
        //  char *lpage = (char *)malloc ( 256 );
        //  strcpy ( lpage, argv[1] );
        //  conn->udata = lpage;
        //}
        //else
        //{
        //  conn->udata = 0;
        //}
        if ( argv[2] ) {
            conn->sess->addVariable ( "uid", argv[2], 0, 0 );
            //printf("Logging User: %s \n", argv[2] );
        } else
        { conn->sess->addVariable ( "uid", "-1", 0, 0 ); }
    }

    return 0;
}

int login_processReq ( Connection *conn ) {
    if ( !conn )
    { return 1; }

    int     rc = 0;
    int     gid = 0, tid = 0, qid = -1;
    char    *error = 0;
    struct  qcmdinfo qcmd;
    char    rcmd[512];
    rcmd[0] = 0;
    qcmd.qcmd[0]  = 0;

    Vector *param = new Vector();
    conn->req.convertPostDataToVector ( param, NULL );

    //conn->resp.setContentLen(0);
    if ( param->size() >= 3 ) {
        gid = atoi ( ( ( *param ) [0] ).c_str() );
        tid = atoi ( ( ( *param ) [1] ).c_str() );
        qid = atoi ( ( ( *param ) [2] ).c_str() );
    } else {
        conn->resp.setContentLen ( 0 );
        sendConnRespHdr ( conn, HTTP_RESP_BADREQ );

        if ( param )
        { delete param; }

        return 0;
    }

    rc = getqcmd ( conn->db, 0, 1, &qcmd );

    conn->resp.setContentType ( "text/xml" );
    sendConnRespHdr ( conn, HTTP_RESP_OK );
    xmlstart ( conn, gid, tid, qid, "-1" );

    if ( rc != 0 || param->size() < 5 ) {
        xmlend ( conn, rc, "Bad Request or Command" );
        //sendConnRespHdr ( conn, HTTP_RESP_BADREQ );
    } else {
        int  m = 0;
        int  n = 0;
        char  *a = rcmd;
        char  *b = qcmd.qcmd;
        mergestring ( a, b, ( char * ) ( ( *param ) [3] ).c_str(), m, n );
        mergestring ( a, b, ( char * ) ( ( *param ) [4] ).c_str(), m, n );
        mergeremaining ( &a[m], &b[n] );

        do {
            rc = sqlite3_exec ( conn->db, rcmd, login_handler, conn, &error );

            if ( rc != SQLITE_OK ) {
                if ( rc == SQLITE_BUSY )
                { PR_Sleep ( 10 ); }
                else {
                    fprintf ( stderr, "%s:SQL error (qtype:0): %s, %s\n", __FUNCTION__, rcmd, error );

                    if ( error ) {
                        sqlite3_free ( error );
                        error = 0;
                    }
                }
            }
        } while ( rc == SQLITE_BUSY );

        fprintf ( stderr, "Login User:%s Status:%s Power:%s\n", ( ( *param ) [0] ).c_str(),
                  rc == SQLITE_OK ? "Success" : "Failure",
                  conn->sess->getVariable ( "auth" ) );
        conn->sess->saveSession ( conn->db );
        conn->sess->updateSaveSession ( conn->db );

        //if( conn->udata )
        //{
        //conn->resp.setLocation ( (char *) conn->udata);
        //printf("INFO: Setting Location: %s\n",(char *) conn->udata);
        //free( conn->udata );
        //}
        if ( rc ) {
            xmlend ( conn, rc, "Unable to login" );
        } else {
            xmlend ( conn, rc, "Logged in" );
        }
    }

    sendRemainderData ( conn );
    conn->len = 0;

    if ( conn->udata ) {
        ChunkHdr *ctemp = ( ChunkHdr * ) conn->udata;
        delete ctemp;
        conn->udata = NULL;
    }

    if ( param )
    { delete param; }

    return 0;
}

