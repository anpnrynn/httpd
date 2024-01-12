#include <httpconn.h>
#include <plugin.h>
#include <httphandlers.h>
#include <http.h>

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

struct PluginHandler pInfo = { 0x00, 0x01, 1, "login.xyz", "", NULL, login_init, login_processReq, login_exit };

int login_init() {
    HttpHandler *hHdlr = HttpHandler::createInstance();

    if ( hHdlr ) {
        fprintf ( stderr, "INFO: Starting Login Plugin\n" );
        hHdlr->addHandler ( pInfo.sname, ( void * ) &pInfo );
    }

    return 0;
}

int login_exit() {
    return 0;
}

int login_handler ( void *data, int argc, char **argv, char **col ) {
    Connection *conn = ( Connection * ) data;

    fprintf ( stderr, "DBUG: login plugin: login.xyz : %s -> %s : %s \n", argv[0], argv[1], argv[2] );

    char public_auth[4] = "1";

    if ( conn ) {
        // add will reset the value if already present
        if ( argv[0] )
        { conn->sess->addVariable ( conn->db, "auth", argv[0], 0, 0 ); }
        else
        { conn->sess->addVariable ( conn->db, "auth", public_auth, 0, 0 ); }

        if ( argv[1] ) {
            char *lpage = ( char * ) malloc ( 256 );
            lpage[0] = 0;
            strcpy ( lpage , argv[1] );
            conn->udata = lpage;
        } else {
            conn->udata = 0;
        }

        if ( argv[2] ) {
            conn->sess->addVariable ( conn->db, "uid", argv[2], 0, 0 );
            fprintf ( stderr, "Logging User: %s \n", argv[2] );
        } else
        { conn->sess->addVariable ( conn->db, "uid", public_auth, 0, 0 ); }
    }

    return 0;
}

int login_processReq ( Connection *conn ) {
    if ( !conn ) {
        fprintf ( stderr, "ERRR: Connection structure is null \n" );
        return 1;
    }

    if ( !conn->db ) {
        fprintf ( stderr, "ERRR: sqlite3 db connection is null \n" );
        return 1;
    }

    int     rc = 0;
    //int     gid = 0, tid = 0, qid = -1;
    char    *error = 0;
    char    rcmd[256] = "";
    char    cmd [128]  = "select power,lpage,id from ulogin where name=\"%s\" and passwd=\"%s\";";

    Vector *param = new Vector();
    conn->req.convertPostDataToVector ( param, NULL );

    conn->resp.setContentLen(0);
    if ( param->size() >= 3 ) {
        //gid = atoi ( ( ( *param ) [0] ).c_str() );
        //tid = atoi ( ( ( *param ) [1] ).c_str() );
        //qid = atoi ( ( ( *param ) [2] ).c_str() );
    } else {
        //conn->resp.setContentLen ( 0 );
        sendConnRespHdr ( conn, HTTP_RESP_BADREQ );
        if ( param )
        { delete param; }

        return 0;
    }

    int isforbidden = 0;
    conn->resp.setContentType ( "text/plain" );

    if ( rc != 0 || param->size() < 5 ) {
        sendConnRespHdr ( conn, HTTP_RESP_BADREQ );
    } else {
        sprintf ( rcmd, cmd, ( char * ) ( ( *param ) [3] ).c_str(),  ( char * ) ( ( *param ) [4] ).c_str() );
        fprintf ( stderr, "DBUG: login plugin: login.xyz : executing sql command : %s \n", rcmd );

        conn->udata = 0;
	//conn->resp.redirect[0] = 0;

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
        //conn->sess->saveSession ( conn->db );
        //conn->sess->updateSaveSession ( conn->db );

        if ( conn->udata ) {
            conn->resp.setLocation ( (char*) conn->udata );
            fprintf ( stderr, "INFO: Setting Location: %s\n", ( char * ) conn->udata );
            free ( conn->udata );
	    conn->udata = 0;
        } else {
            conn->resp.setLocation ( (char*)"403.sthtml" );
            isforbidden = 1;
            fprintf ( stderr, "INFO: Setting Location: 403.sthtml\n" );
        }
    }

    if ( isforbidden )
    { sendConnRespHdr ( conn, HTTP_RESP_FORB ); }
    else
    { sendConnRespHdr ( conn, HTTP_RESP_REDIRECT ); }

    sendRemainderData ( conn );
    conn->len = 0;

    if ( param )
    { delete param; }

    return 0;
}

