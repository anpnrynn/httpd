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

int plugin_init();
int plugin_exit();
int p2_processReq ( Connection *conn );

struct PluginHandler pInfo = { 0x00, 0x01, 1, "p2", "", NULL, plugin_init, p2_processReq, plugin_exit };

int plugin_init() {
    HttpHandler *hHdlr = HttpHandler::createInstance();

    if ( hHdlr )
    { hHdlr->addHandler ( pInfo.sname, ( void * ) &pInfo ); }

    return 0;
}

int plugin_exit() {
    return 0;
}

int p2_processReq ( Connection *conn ) {
    if ( !conn )
    { return 1; }

    int     rc = 0;
    int     gid, tid, qid, power = 3;
    char    *error = 0;
    struct  qcmdinfo qcmd;
    char    rcmd[512];
    rcmd[0] = 0;
    qcmd.qcmd[0]  = 0;

    Vector *param = new Vector();
    conn->req.convertGetDataToVector ( param );

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

    conn->resp.setContentType ( "text/xml" );
    sendConnRespHdr ( conn, HTTP_RESP_OK );
    xmlstart ( conn, gid, tid, qid, conn->sess->getVariable ( "uid" ) );

    power = atoi ( conn->sess->getVariable ( "auth" ) );
    //power = 3;
    rc = getqcmd ( conn->db, qid, power, &qcmd );



    if ( rc != 0 ) {
        xmlend ( conn, rc, "Bad Command" );
    } else {
        fprintf(stderr, "ReqQuery: gid=%d, tid=%d, qid=%d, power=%d \n", gid, tid, qid, power );
        fprintf(stderr, "QCmd:  nParam:%d, qcmdPNum:%d, qcmdQtype:%d, qcmdKey:%d, qcmd:%s \n", param->size(), qcmd.pnum, qcmd.qtype, qcmd.key, qcmd.qcmd );

        if ( qcmd.qtype == 0 ) {
            rc = sqlexecute ( conn->db, qcmd.qcmd, sqlhandlertype2, conn, NULL );
        } else if ( qcmd.qtype == 1 ) {
            rc = sqlexecute ( conn->db, qcmd.qcmd, sqlhandler, conn, NULL );
        } else if ( qcmd.qtype == 2 ) {
            int  i = 3;
            int  j = 0;
            int  m = 0;
            int  n = 0;
            char  *a = rcmd;
            char  *b = qcmd.qcmd;
            rc = sqlexecute ( conn->db, "BEGIN IMMEDIATE;", NULL, NULL, NULL );

            while ( ( unsigned int ) ( i + qcmd.pnum ) <= param->size() ) {
                m = 0;
                n = 0;

                for ( j = 0; j < qcmd.pnum; j++ ) {
                    mergestring ( a, b, ( char * ) ( ( *param ) [i + j] ).c_str(), m, n );
                }

                mergeremaining ( &a[m], &b[n] );
                fprintf(stderr, "RCmd: %s \n", rcmd );
                rc = sqlexecute ( conn->db, rcmd, NULL, NULL, NULL );
                i += qcmd.pnum;
            }

            xmllastrowid ( conn, qcmd.qid, ( unsigned long long ) sqlite3_last_insert_rowid ( conn->db ) );
            rc = sqlexecute ( conn->db, "COMMIT;", NULL, NULL, NULL );
        } else if ( qcmd.qtype == 3 ) {
            int  i = 3;
            int  j = 0;
            int  m = 0;
            int  n = 0;
            char  *a = rcmd;
            char  *b = qcmd.qcmd;

            //rc = sqlexecute ( conn->db, "BEGIN IMMEDIATE;", NULL, NULL, NULL );
            if ( param->size() <= ( unsigned int ) ( i + qcmd.pnum ) ) {
                for ( j = 0; j < qcmd.pnum; j++ ) {
                    mergestring ( a, b, ( char * ) ( ( *param ) [i + j] ).c_str(), m, n );
                }

                mergeremaining ( &a[m], &b[n] );
                fprintf(stderr, "RCmd: %s \n", rcmd );
                rc = sqlexecute ( conn->db, rcmd, sqlhandler, conn, &qcmd );
            }

            //rc = sqlexecute ( conn->db, "COMMIT;", NULL, NULL , NULL);
        } else if ( qcmd.qtype == 5 ) {
            rc = sqlexecute ( conn->db, rcmd, sqlhandlertype3, conn, NULL );
        }


        if ( error ) {
            xmlend ( conn, rc, error );
            sqlite3_free ( error );
        } else {
            xmlend ( conn, rc, "success" );
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

