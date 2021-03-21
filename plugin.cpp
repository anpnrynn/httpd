#include <plugin.h>

void registerPlugin ( PluginHandler *pInfo ) {
    HttpHandler *httpHdlr = HttpHandler::createInstance();

    if ( httpHdlr ) {
        httpHdlr->addHandler ( pInfo->sname, pInfo );
    }
}

void deregisterPlugin ( PluginHandler *pInfo ) {
    HttpHandler *httpHdlr = HttpHandler::createInstance();

    if ( httpHdlr ) {
        httpHdlr->delHandler ( pInfo->sname );
    }
}

int   qcmdreader ( void *udata, int argc, char **argv, char **col ) {
    struct qcmdinfo *qcmd = ( struct qcmdinfo * ) udata;

    if ( qcmd ) {
        if ( argv[0] )
        { qcmd->pnum = atoi ( argv[0] ); }
        else
        { qcmd->pnum = 0; }

        if ( argv[1] )
        { qcmd->qtype = atoi ( argv[1] ); }
        else
        { qcmd->qtype = 0; }

        if ( argv[2] )
        { qcmd->key = atoi ( argv[2] ); }
        else
        { qcmd->key = 0; }

        if ( argv[3] )
        { strcpy ( qcmd->qcmd, argv[3] ); }
        else
        { qcmd->qcmd[0] = 0; }

        //printf("Qcmd: %d, %d, %s \n",qcmd->pnum, qcmd->qtype, qcmd->qcmd);
    }

    return 0;
}

int   getqcmd  ( sqlite3 *db, int qid, int power, struct qcmdinfo *qcmd ) {
    char query[128];
    int rc;
    char *error = NULL;

    if ( qcmd && db ) {
        if ( power <= 0 )
        { power = 1; }

        qcmd->qid = qid;
        sprintf ( query, "select paramnum, qtype, key, qcmd from qcmds where qid=%d and powerreq<=%d;", qid, power );

        //printf ("Qcmd Query: %s \n", query );
        do {
            rc = sqlite3_exec ( db, query, qcmdreader, qcmd, &error );

            if ( rc != SQLITE_OK ) {
                if ( rc == SQLITE_BUSY )
                { PR_Sleep ( 10 ); }
                else {
                    if ( error )
                    { sqlite3_free ( error ); }

                    return 1;
                }
            }
        } while ( rc == SQLITE_BUSY );
    }

    return 0;
}

int   flushdata        ( Connection *conn, unsigned char **start, unsigned char **clen ) {
    if ( ( conn->len = sendConnectionData ( conn->socket, conn->buf, SMALLBUF ) ) > SMALLBUF )
    { return 0; }
    else
    { return createChunk ( conn->buf, conn->len, start, clen ); }
}

int appendrespstring    ( Connection *conn, char *buf ) {
    ChunkHdr        *cHdr = 0;
    unsigned char  *start = 0;
    unsigned char   *clen = 0;
    unsigned int    space = 0;
    unsigned char *incomp = 0;

    if ( buf ) {
        if ( !conn->udata ) {
            cHdr = new ChunkHdr();
            conn->udata = cHdr;
            cHdr->getValues ( &start, &clen, &space );
        } else {
            cHdr = ( ChunkHdr * ) conn->udata ;
            cHdr->getValues ( &start, &clen, &space );
        }

        if ( conn->len == 0 || space == SMALLBUF ) {
            space = createChunk ( conn->buf, conn->len, &start, &clen );
        }

        if ( space == 0 ) {
            if ( ( conn->len = sendConnectionData ( conn->socket, conn->buf, SMALLBUF ) ) > SMALLBUF )
            { return 1; }
            else
            { space = createChunk ( conn->buf, conn->len, &start, &clen ); }
        }

        appendChunkString ( &start, conn->len, space, ( unsigned char * ) buf, &incomp );

        if ( space == 0 ) {
            if ( ( conn->len = sendConnectionData ( conn->socket, conn->buf, SMALLBUF ) ) > SMALLBUF )
            { return 1; }
            else
            { space = createChunk ( conn->buf, conn->len, &start, &clen ); }
        }

        if ( incomp ) {
            unsigned char *data = incomp;
            appendChunkString ( &start, conn->len, space, data, &incomp );

            if ( space == 0 ) {
                if ( ( conn->len = sendConnectionData ( conn->socket, conn->buf, SMALLBUF ) ) > SMALLBUF )
                { return 1; }
                else
                { space = createChunk ( conn->buf, conn->len, &start, &clen ); }
            }
        }

        cHdr->setValues ( start, clen, space );
        return 0;
    }

    return 1;
}

int   xmlstart         ( Connection *conn, int gid, int tid, int qid, char *uid ) {
    if ( conn ) {
        char temp[128];
        sprintf ( temp, "<?xml version=\"1.0\"?><response gid=\"%d\" tid=\"%d\" qid=\"%d\" uid=\"%s\"><data>",
                  gid, tid, qid, uid ? uid : "-1" );
        appendrespstring ( conn, temp );
        return 0;
    }

    return 1;
}

int   xmlrsetstart     ( Connection *conn ) {
    if ( conn ) {
        appendrespstring ( conn, ( char * ) "<!--" );
    }

    return 0;
}

int   xmlrsetend       ( Connection *conn ) {
    if ( conn ) {
        appendrespstring ( conn, ( char * ) "-->" );
    }

    return 0;
}

int   xmlend           ( Connection *conn, int status, char *error ) {
    if ( conn ) {
        char temp[256];
        sprintf ( temp, "</data><status rc=\"%d\" error=\"%s\" /></response>", status, error ? error : "" );
        appendrespstring ( conn, temp );
    }

    return 0;
}

int   xmllastrowid     ( Connection *conn, int qid, unsigned long long key ) {
    if ( conn ) {
        char temp[256];
        sprintf ( temp, "<rowid qid=\"%d\" last=\"%lld\" />", qid, key );
        appendrespstring ( conn, temp );
    }

    return 0;
}

int sqlexecute ( sqlite3 *db, const char *sql,
                 int ( *callback ) ( void*, int, char**, char** ), void *data, struct qcmdinfo *qcmd ) {
    int  rc = 0;
    char *error = NULL;

    do {
        rc = sqlite3_exec ( db, sql, callback, data, &error );

        if ( rc != SQLITE_OK ) {
            if ( rc == SQLITE_BUSY )
            { PR_Sleep ( 10 ); }
            else {
                if ( error ) {
                    fprintf ( stderr, "%s:SQL error (%s): %s\n", __FUNCTION__, sql, error );
                    sqlite3_free ( error );
                    error = 0;
                } else {
                    fprintf ( stderr, "%s:SQL error : %s\n", __FUNCTION__, sql );
                }
            }
        }
    } while ( rc == SQLITE_BUSY );

    if ( qcmd && qcmd->key ) {
        //printf("Reached here \n");
        //sleep(2);
        xmllastrowid ( ( Connection * ) data, qcmd->qid, ( unsigned long long ) sqlite3_last_insert_rowid ( db ) );
        //unsigned long long value = sqlite3_last_insert_rowid(db);
        //xmllastrowid( (Connection *) data, qcmd->qid, value );
        //printf("Last Insert Row id = '%lld' \n", value);
    }

    return rc;
}

int sqlhandler ( void *udata, int argc, char **argv, char **colName ) {
    unsigned char *start  = 0;
    unsigned char *clen   = 0;
    unsigned int   space  = 0;
    unsigned char *incomp = 0;
    ChunkHdr *cHdr        = 0;
    const char *row = "<r v=\"";
    const char *rowend = "\"/>";
    //printf(" Reached here \n");

    Connection *conn = ( Connection * ) udata;

    if ( conn ) {
        if ( !conn->udata ) {
            cHdr = new ChunkHdr();
            conn->udata = cHdr;
            cHdr->getValues ( &start, &clen, &space );
        } else {
            cHdr = ( ChunkHdr * ) conn->udata ;
            cHdr->getValues ( &start, &clen, &space );
        }

        int i = 0;

        if ( conn->len == 0 || space == SMALLBUF ) {
            space = createChunk ( conn->buf, conn->len, &start, &clen );
        }

        if ( space == 0 ) {
            if ( ( conn->len = sendConnectionData ( conn->socket, conn->buf, SMALLBUF ) ) > SMALLBUF )
            { return 1; }
            else
            { space = createChunk ( conn->buf, conn->len, &start, &clen ); }
        }

        /*appendChunkChar ( &start, conn->len, space, '{' );

        if( space == 0 )
        {
            if( (conn->len = sendConnectionData(conn->socket, conn->buf, SMALLBUF) ) > SMALLBUF )
                return 1;
            else
                space = createChunk ( conn->buf, conn->len, &start , &clen );
        } */

        unsigned char *data = ( unsigned char * ) row;
        appendChunkString ( &start, conn->len, space, data, &incomp );

        if ( space == 0 ) {
            if ( ( conn->len = sendConnectionData ( conn->socket, conn->buf, SMALLBUF ) ) > SMALLBUF )
            { return 1; }
            else
            { space = createChunk ( conn->buf, conn->len, &start, &clen ); }
        }

        if ( incomp ) {
            data = incomp;
            appendChunkString ( &start, conn->len, space, data, &incomp );
        }

        for ( ; i < argc; i++ ) {
            if ( argv[i] ) {
                data = ( unsigned char * ) argv[i];
                appendChunkString ( &start, conn->len, space, data, &incomp );

                if ( space == 0 ) {
                    if ( ( conn->len = sendConnectionData ( conn->socket, conn->buf, SMALLBUF ) ) > SMALLBUF )
                    { return 1; }
                    else
                    { space = createChunk ( conn->buf, conn->len, &start, &clen ); }
                }

                if ( incomp ) {
                    data = incomp;
                    appendChunkString ( &start, conn->len, space, data, &incomp );
                }
            }

            if ( space == 0 ) {
                if ( ( conn->len = sendConnectionData ( conn->socket, conn->buf, SMALLBUF ) ) > SMALLBUF )
                { return 1; }
                else
                { space = createChunk ( conn->buf, conn->len, &start, &clen ); }
            }

            appendChunkChar ( &start, conn->len, space, '|' );

            if ( space == 0 ) {
                if ( ( conn->len = sendConnectionData ( conn->socket, conn->buf, SMALLBUF ) ) > SMALLBUF )
                { return 1; }
                else
                { space = createChunk ( conn->buf, conn->len, &start, &clen ); }
            }
        }

        data = ( unsigned char * ) rowend;
        appendChunkString ( &start, conn->len, space, data, &incomp );

        if ( space == 0 ) {
            if ( ( conn->len = sendConnectionData ( conn->socket, conn->buf, SMALLBUF ) ) > SMALLBUF )
            { return 1; }
            else
            { space = createChunk ( conn->buf, conn->len, &start, &clen ); }
        }

        if ( incomp ) {
            data = incomp;
            appendChunkString ( &start, conn->len, space, data, &incomp );
        }

        /* appendChunkChar ( &start, conn->len, space, '}' );
        if( space == 0 )
        {
            if( (conn->len = sendConnectionData(conn->socket, conn->buf, SMALLBUF) ) > SMALLBUF )
                return 1;
            else
                space = createChunk ( conn->buf, conn->len, &start , &clen );
        } */
        cHdr->setValues ( start, clen, space );
        return 0;
    } else {
        fprintf ( stderr, "ERRR: Connection data not present \n" );
        return 1;
    }
}


int sqlhandlertype2 ( void *udata, int argc, char **argv, char **colName ) {
    unsigned char *start  = 0;
    unsigned char *clen   = 0;
    unsigned int   space  = 0;
    unsigned char *incomp = 0;
    ChunkHdr       *cHdr  = 0;
    Connection *conn = ( Connection * ) udata;

    if ( conn ) {
        if ( !conn->udata ) {
            cHdr = new ChunkHdr();
            conn->udata = cHdr;
            cHdr->getValues ( &start, &clen, &space );
        } else {
            cHdr = ( ChunkHdr * ) conn->udata ;
            cHdr->getValues ( &start, &clen, &space );
        }

        if ( conn->len == 0 || space == SMALLBUF ) {
            space = createChunk ( conn->buf, conn->len, &start, &clen );
        }

        if ( space == 0 )
        { space = flushdata ( conn, &start, &clen ); }

        appendChunkChar ( &start, conn->len, space, '<' );

        if ( space == 0 )
        { space = flushdata ( conn, &start, &clen ); }

        if ( argv[0] ) {
            unsigned char *data = ( unsigned char * ) argv[0];
            appendChunkString ( &start, conn->len, space, data, &incomp );

            if ( space == 0 )
            { space = flushdata ( conn, &start, &clen ); }

            if ( incomp ) {
                data = incomp;
                appendChunkString ( &start, conn->len, space, data, &incomp );
            }
        }

        if ( space == 0 )
        { space = flushdata ( conn, &start, &clen ); }


        if ( argv[1] ) {
            appendChunkChar ( &start, conn->len, space, ' ' );

            if ( space == 0 )
            { space = flushdata ( conn, &start, &clen ); }

            appendChunkChar ( &start, conn->len, space, 'v' );

            if ( space == 0 )
            { space = flushdata ( conn, &start, &clen ); }

            appendChunkChar ( &start, conn->len, space, '=' );

            if ( space == 0 )
            { space = flushdata ( conn, &start, &clen ); }

            appendChunkChar ( &start, conn->len, space, '"' );

            if ( space == 0 )
            { space = flushdata ( conn, &start, &clen ); }

            unsigned char *data = ( unsigned char * ) argv[1];
            appendChunkString ( &start, conn->len, space, data, &incomp );

            if ( space == 0 )
            { space = flushdata ( conn, &start, &clen ); }

            if ( incomp ) {
                data = incomp;
                appendChunkString ( &start, conn->len, space, data, &incomp );
            }

            appendChunkChar ( &start, conn->len, space, '"' );

            if ( space == 0 )
            { space = flushdata ( conn, &start, &clen ); }
        }

        if ( argv[2] ) {
            appendChunkChar ( &start, conn->len, space, ' ' );

            if ( space == 0 )
            { space = flushdata ( conn, &start, &clen ); }

            appendChunkChar ( &start, conn->len, space, 't' );

            if ( space == 0 )
            { space = flushdata ( conn, &start, &clen ); }

            appendChunkChar ( &start, conn->len, space, '=' );

            if ( space == 0 )
            { space = flushdata ( conn, &start, &clen ); }

            appendChunkChar ( &start, conn->len, space, '"' );

            if ( space == 0 )
            { space = flushdata ( conn, &start, &clen ); }

            unsigned char *data = ( unsigned char * ) argv[2];
            appendChunkString ( &start, conn->len, space, data, &incomp );

            if ( space == 0 )
            { space = flushdata ( conn, &start, &clen ); }

            if ( incomp ) {
                data = incomp;
                appendChunkString ( &start, conn->len, space, data, &incomp );
            }

            appendChunkChar ( &start, conn->len, space, '"' );

            if ( space == 0 )
            { space = flushdata ( conn, &start, &clen ); }
        }

        if ( space == 0 )
        { space = flushdata ( conn, &start, &clen ); }

        appendChunkChar ( &start, conn->len, space, '/' );

        if ( space == 0 )
        { space = flushdata ( conn, &start, &clen ); }

        appendChunkChar ( &start, conn->len, space, '>' );

        if ( space == 0 )
        { space = flushdata ( conn, &start, &clen ); }

        cHdr->setValues ( start, clen, space );
        return 0;
    } else {
        fprintf ( stderr, "ERRR: Connection data not present \n" );
        return 1;
    }
}

//max values
int sqlhandlertype3 ( void *udata, int argc, char **argv, char **colName ) {
    unsigned char *start  = 0;
    unsigned char *clen   = 0;
    unsigned int   space  = 0;
    unsigned char *incomp = 0;
    ChunkHdr *cHdr        = 0;

    Connection *conn = ( Connection * ) udata;

    if ( conn ) {
        if ( !conn->udata ) {
            cHdr = new ChunkHdr();
            conn->udata = cHdr;
            cHdr->getValues ( &start, &clen, &space );
        } else {
            cHdr = ( ChunkHdr * ) conn->udata ;
            cHdr->getValues ( &start, &clen, &space );
        }

        if ( conn->len == 0 || space == SMALLBUF ) {
            space = createChunk ( conn->buf, conn->len, &start, &clen );
        }

        if ( space == 0 ) {
            if ( ( conn->len = sendConnectionData ( conn->socket, conn->buf, SMALLBUF ) ) > SMALLBUF )
            { return 1; }
            else
            { space = createChunk ( conn->buf, conn->len, &start, &clen ); }
        }

        if ( argv[0] ) {
            unsigned char *data = ( unsigned char * ) argv[0];
            appendChunkString ( &start, conn->len, space, data, &incomp );

            if ( space == 0 ) {
                if ( ( conn->len = sendConnectionData ( conn->socket, conn->buf, SMALLBUF ) ) > SMALLBUF )
                { return 1; }
                else
                { space = createChunk ( conn->buf, conn->len, &start, &clen ); }
            }

            if ( incomp ) {
                data = incomp;
                appendChunkString ( &start, conn->len, space, data, &incomp );
            }
        }

        if ( space == 0 ) {
            if ( ( conn->len = sendConnectionData ( conn->socket, conn->buf, SMALLBUF ) ) > SMALLBUF )
            { return 1; }
            else
            { space = createChunk ( conn->buf, conn->len, &start, &clen ); }
        }

        cHdr->setValues ( start, clen, space );
        return 0;
    } else {
        fprintf ( stderr, "ERRR: Connection data not present \n" );
        return 1;
    }
}
