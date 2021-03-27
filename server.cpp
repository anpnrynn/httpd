#include <iostream>
//#include <mozhdr.h>
#include <prwrapper.h>
#include <sqlite3.h>
#include <httpconn.h>
//#include <ifs.h>
#include <httpcodes.h>
#include <mimetypes.h>
#include <session.h>
#include <threadmgr.h>
#include <plugin.h>
#include <pass.h>
#include <defines.h>
#include <malloc.h>

#undef USE_MINIUPNP
#ifdef USE_MINIUPNP
#include <upnp.h>
#endif


#ifndef USE_PTHREAD
#include <prthread.h>
#else
#include <pthread.h>
#endif


#ifdef LINUX_BUILD
#include <signal.h>
#endif

using namespace std;
PRLibrary *lib[MAXPLUGINS];

int srvSocket = 0;
int clientSocket = 0;
PRFileDesc *srv = &srvSocket, *client = &clientSocket;
unsigned short int SRVPORT = 0;
bool isBound = false;

//Clean up operations
void performCleanupOperations() {
    ThreadMgr *tMgr = ThreadMgr::createInstance();

    if ( isBound && tMgr )
    { tMgr->stopThreads(); }
}


//SIGNAL HANDLER
void signalStop ( int signal ) {
    fprintf ( stderr, "INFO: Received signal to terminate from user : %d \n", signal );

    if ( srv )
    { PR_Close ( srv ); }

    performCleanupOperations();
    exit ( 0 );
}


void clearPlugins() {
    int i;

    for ( i = 0; i < MAXPLUGINS; i++ ) {
        lib[i] = NULL;
    }
}

void unloadPlugins() {
}

static int aclCount = 0;
static unsigned int aclIp[MAXCLIENTS];
int aclLoad ( void *udata, int argc, char **argv, char **col ) {
    if ( argv[0] ) {
        aclIp[aclCount] = ( unsigned int ) atoi ( argv[0] );

        if ( aclIp[aclCount] )
        { aclCount++; }
    }

    return 0;
}

static int libCount = 0;
extern int login_init();
int pluginLoader ( void *udata, int argc, char **argv, char **col ) {
    char lName[256];

    if ( argv[0] && argv[1] && atoi ( argv[1] ) ) {
#ifdef LINUX_BUILD
        sprintf ( lName, "/usr/lib/lib%s.so", argv[0] );
        lib[libCount] = PR_LoadLibrary ( lName );

        if ( !lib[libCount] ) {
            fprintf ( stderr, "WARN: Unable to load library:%s \n", lName );
            sprintf ( lName, "/usr/local/lib/lib%s.so", argv[0] );
            lib[libCount] = PR_LoadLibrary ( lName );

            if ( !lib[libCount] ) {
                fprintf ( stderr, "WARN: Unable to load library:%s \n", lName );
                sprintf ( lName, "./lib%s.so", argv[0] );
                lib[libCount] = PR_LoadLibrary ( lName );

                if ( !lib[libCount] ) {
                    fprintf ( stderr, "ERRR: Library:%s NOT FOUND\n", lName );
                    lib[libCount] = NULL;
                }
            }
        }

        if ( lib[libCount] ) {
            fprintf ( stderr, "Loaded Library:%s \n", lName );
            libCount++;
        }

#else
        sprintf ( lName, "%s.dll", argv[0] );
        lib[libCount] = PR_LoadLibrary ( lName );

        if ( !lib[libCount] ) {
            fprintf ( stderr, "ERRR: Library: %s NOT FOUND\n", lName );
        } else {
            fprintf ( stderr, "Loaded Library: %s\n", lName );
            libCount++;
        }

#endif
    }

    return 0;
}


int loadInfo() {
    sqlite3 *db = NULL;
    //int rc = sqlite3_open(INFO_STORE, &db, getEncryptionStructure( ENC_KEY_DATABASE ) );
    int rc = sqlite3_open ( INFO_STORE, &db );
    char *error = NULL;
    //TODO: removing login plugin
    //login_init ();

    if ( rc != SQLITE_OK ) {
        fprintf ( stderr, "ERRR: Unable to open db, Corrupted ? \n" );
        sqlite3_close ( db );
        db = NULL;
        return 1;
    }

    rc = sqlite3_exec ( db, "select name,active from plugins;", pluginLoader, NULL, &error );

    if ( rc != SQLITE_OK ) {
        fprintf ( stderr, "ERRR: Unable to load necessary plugins \n" );
        sqlite3_close ( db );
        db = NULL;
        return 2;
    }

    rc = sqlite3_exec ( db, "select ipaddr from acl;", aclLoad, NULL, &error );

    if ( rc != SQLITE_OK ) {
        fprintf ( stderr, "ERRR: Unable to load necessary plugins \n" );
        sqlite3_close ( db );
        db = NULL;
        return 3;
    }

    sqlite3_close ( db );
    db = NULL;
    return 0;
}

void initPlugins() {
    int i;

    for ( i = 0; i < libCount; i++ ) {
        if ( lib[i] ) {
            //INITFUNC func = (INITFUNC) PR_FindSymbol(lib[i], "plugin_init");
            PluginHandler *plugin = ( PluginHandler * ) PR_FindSymbol ( lib[i], "pInfo" );

            if ( plugin ) {
                plugin->init();
                //if ( 0 == plugin->init() )
                //{
                //  fprintf(stderr,"INFO: Plugin Initialized \n");
                //}
            } else {
                fprintf ( stderr, "ERRR: Unable to locate symbol in object code \n" );
            }
        } else {
            fprintf ( stderr, "ERRR: Plugin Data Structure Empty \n" );
        }
    }
}

//int installTheNeededStuff(void *ifs)
int installTheNeededStuff() {
    char cmd[1024], buf[256], *error;

    int rc = 0;
    sqlite3 *db;
    //rc = sqlite3_open(INFO_STORE, &db, getEncryptionStructure( ENC_KEY_DATABASE ) );
    rc = sqlite3_open ( INFO_STORE, &db );

    if ( rc ) {
        fprintf ( stderr, "ERRR: Unable to open db, Corrupted ? \n" );
        sqlite3_close ( db );
        db = NULL;
        return 2;
    }

    //FileIndexData *fData= NULL;
    //RFileDesc *file = PR_Open( "ins3.sh", &fData );
    int filefd = 0;
    PRFileDesc *file = &filefd;
    *file = PR_Open ( "install.sql", PR_RDONLY, 0 );


    //if( !file || !fData )
    if ( !file && *file == -1 ) {
        fprintf ( stderr, "ERRR: Unable to open install.sql file \n" );
        return 3;
    }

    int i = 0, j = 0, bRead = 0;

    rc = sqlexecute ( db, "BEGIN IMMEDIATE;", NULL, NULL, NULL );

    //while ( ( bRead = ifs->ifsRead( file, fData, (unsigned char *)&buf, 256, &offset ) ) > 0 )
    while ( ( bRead = PR_Read ( file, ( unsigned char * ) &buf, 256 ) ) > 0 ) {
        i = 0;

        while ( i < bRead ) {
            if ( buf[i] == '\r' )
            { cmd[j] = 0; }
            else if ( buf[i] == '\n' ) {
                cmd[j] = 0;
                error  = NULL;
                fprintf ( stderr, "INFO: Running Cmd '%s' \n", cmd );
                rc = sqlite3_exec ( db, cmd, NULL, NULL, &error );

                if ( rc != SQLITE_OK ) {
                    if ( error ) {
                        sqlite3_free ( error );
                    }

                    fprintf ( stderr, "ERRR: Unable to run command '%s'\n", cmd );
                    sqlite3_close ( db );
                    PR_Close ( file );
                    return 4;
                }

                j = 0;
            } else
            { cmd[j++] = buf[i]; }

            i++;
        }
    }

    rc = sqlexecute ( db, "COMMIT;", NULL, NULL, NULL );
    sqlite3_close ( db );
    PR_Close ( file );
    fprintf ( stderr, "INFO: Done installing prerequisites \n" );
//#endif
    return 0;
}


#ifdef USE_MINIUPNP
#ifndef USE_PTHREAD
void upnpThread ( void * data ) {
#else
void* upnpThread ( void * data ) {
#endif
    std::cerr << "DBUG: Upnp thread started " << std::endl;
    tr_upnp *upnp = tr_upnpInit();

    while ( 1 ) {
        int err = tr_upnpPulse ( upnp, SRVPORT, true, true );

        if ( err != TR_PORT_MAPPED ) {
            PR_Sleep ( 1000 );

            if ( err == TR_PORT_ERROR ) {
                std::cerr << "ERRR: Upnp port forwarding error... exiting " << std::endl;
                PR_Sleep ( 100000 );
                break;
            }

            std::cerr << "DBUG: Upnp thread short sleep" << std::endl;
            continue;
        } else {
            PR_Sleep ( 1000000 );
        }

        std::cerr << "DBUG: Upnp thread sleeping " << std::endl;
    }

    tr_upnpClose ( upnp );
#ifdef USE_PTHREAD
    return 0;
#endif
}
#endif

extern int MAX_THREADS;

int main ( int argc, char *argv[] ) {
    bool  isRestart = false;
    short firstTime = 0;
    
    fclose(stderr);
    stderr = NULL;
    stderr = fopen(LOGFILE, "a+");
    
    if(!stderr)
        exit(56);
    
    if ( argc < 2 ) {
        fprintf ( stderr, "%s Exited \n", argv[0] );
        fprintf ( stderr, "Format: %s port count \n", argv[0] );
        fprintf ( stderr, "port  - Server port number to use \n" );
        fprintf ( stderr, "count - Number of threads to launch \n" );
        exit ( 1 );
    } else {
        SRVPORT = atoi ( argv[1] );
    }

    if ( argc == 3 ) {
        fprintf ( stderr, "INFO: Received threads count: %s \n", argv[2] );
        MAX_THREADS = atoi ( argv[2] );

        if ( MAX_THREADS > MAX_POSSIBLE_THREADS ) {
            fprintf ( stderr, "ERRR: Cannot create more than 30 threads \n" );
            exit ( 1 );
        } else {
            if ( MAX_THREADS <= 0 ) {
                fprintf ( stderr, "ERRR: Number of threads cannot be zero or less than zero \n" );
                exit ( 1 );
            }
        }
    }

    fprintf ( stderr, "INFO: Proceeding with the boot \n" );
#if 0

#ifdef USE_MINIUPNP


#ifndef USE_PTHREAD
    PRThread* upnpThreadPtr = PR_CreateThread ( PR_SYSTEM_THREAD, upnpThread, 0,
                              PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD, PR_JOINABLE_THREAD, 8388608 );

    if ( upnpThreadPtr == 0 ) {
        fprintf ( stderr, "ERRR: upnp thread creation failed \n" );
    }

    fprintf ( stderr, "INFO: Started Upnp Thread... \n" );
#else
    pthread_t upnpThreadPtr;
    int rc = pthread_create ( &upnpThreadPtr, 0, upnpThread, 0 );

    if ( rc != 0 ) {
        fprintf ( stderr, "ERRR: upnp thread creation failed \n" );
    }

    PR_Sleep ( 10 );
#endif

#endif

#ifdef LINUX_BUILD
    signal ( SIGINT, signalStop );
    signal ( SIGSTOP, signalStop );
    signal ( SIGABRT, signalStop );
#endif

#endif

start:
    PRNetAddr  srvAddr;
    PRNetAddr  clientAddr;

    fprintf ( stderr, "INFO: Creating socket \n" );

    *srv = PR_NewTCPSocket();
    srvAddr.sin_family  = PR_AF_INET;
    srvAddr.sin_addr.s_addr     = 0x00000000;
    srvAddr.sin_port   = PR_htons ( SRVPORT );
    fprintf ( stderr, "INFO: Socket created successfully \n" );
    fflush ( stdout );

    //if( PR_Bind(srv, &srvAddr) ==  PR_FAILURE )
    int count = 0;

    while ( bind ( *srv, ( const sockaddr * ) &srvAddr, sizeof ( sockaddr_in ) ) !=  0 ) {
        fprintf ( stderr, "ERRR: Unable to Bind \n" );
        PR_Cleanup();
        PR_Sleep ( 1000 );

        if ( count > 1000 ) {
            return 1;
        } else {
            count++;
        }

        //return 1;
    }

    isBound = true;

    fprintf ( stderr, "INFO: Bound successfully \n" );

    if ( PR_Listen ( srv, 20 ) == PR_FAILURE ) {
        fprintf ( stderr, "ERRR: Unable to setup backlog \n" );
        PR_Cleanup();
        return 2;
    }

    fprintf ( stderr, "INFO: Listening successfully \n" );
    //PRSocketOptionData  opt;
    //opt.option = PR_SockOpt_Nonblocking;
    //opt.value.non_blocking = true;

    //if( PR_SetSocketOption(srv, &opt) == PR_FAILURE )
    if ( fcntl ( *srv, F_SETFL, O_NONBLOCK ) != 0 ) {
        fprintf ( stderr, "ERRR: Unable to set socket option \n" );
        PR_Cleanup();
        return 3;
    }

    fprintf ( stderr, "INFO: Non blocking successfully \n" );
    fflush ( stdout );
#if 0

    if ( 0 != acquireEncryptionKeys ( &firstTime ) ) {
        fprintf ( stderr, "ERRR: Acquiring Encryption Keys Failed \n" );
        PR_Cleanup();
        return 4;
    }

#endif

    //Ifs *ifs = Ifs::createInstance( FILE_STORE, true);
    //struct Sfea *sfea = getEncryptionStructure( ENC_KEY_FILESTORE );
    //Ifs *ifs = Ifs::createInstance( sfea, FILE_STORE );
    //void *ifs = 0;
    HttpSessionMgr *hSMgr = NULL;

    if ( ! isRestart ) {
        isRestart = true;

        int infostoreFd = 0;
        PRFileDesc *infoStore = &infostoreFd;
        *infoStore = PR_Open ( INFO_STORE, PR_RDONLY, 0 );

        if ( infoStore && *infoStore != -1 ) {
            fprintf ( stderr, "WARN: Database present \n" );
            PR_Close ( infoStore );

            if ( firstTime ) {
                fprintf ( stderr, "WARN: Database key file not present, but database present \n" );
                fprintf ( stderr, "WARN: Please backup the database to another location and restart this program \n" );
                PR_Cleanup();
                return 10;
            }
        } else {
            fprintf ( stderr, "WARN: Database not present, installing the needed stuff \n" );

            //if( firstTime )
            //{
            /*
             *  If first time (Setup the datastore)
             */
            if ( 0 != installTheNeededStuff() ) {
                fprintf ( stderr, "ERRR: Unable to perform internal installation \n" );
                PR_Cleanup();
                return 11;
            }

            //}
            //else
            //{
            //  fprintf(stderr,"WARN: The database is missing, but this isn't the first time \n");
            //printf("WARN: Please backup the keyfile to another location, and restart this program \n");
            //  PR_Cleanup();
            //  return 12;
            //}
        }

        fprintf ( stderr, "WARN: Starting session manager \n" );
        hSMgr = HttpSessionMgr::createInstance();
        /*
         *  Restore persistent sessions
         */
#if 1

        if ( hSMgr->loadStoredSessions() != 0 ) {
            fprintf ( stderr, "ERRR: Unable to restore session data \n" );
            return 13;
        } else {
            fprintf ( stderr, "INFO: Reading stored data completed \n" );
        }

#endif
        fprintf ( stderr, "WARN: Clearing plugins \n" );
        clearPlugins();
        loadInfo();
        initPlugins();
        fprintf ( stderr, "WARN: Starting plugins \n" );
#ifdef LINUX_BUILD
        malloc_trim ( 0 );
#endif
    }

    setupStatusCodes();
    setupContentTypes();
    ThreadMgr *tMgr = ThreadMgr::createInstance();

    if ( tMgr )
    { tMgr->startThreads(); }


    //TODO:
    //start upnp and natpmp thread here.

    Connection *conn[MAXCLIENTS];
    PRPollDesc pollfds[MAXCLIENTS];
    bool exitMain = false;
    bool shrink   = true;
    int  nClients = 1;
    int  offset   = 1;
    int  shift    = 0;
    int  retVal   = 0;
    int  i;

    for ( i = 0; i < MAXCLIENTS; i++ ) {
        conn[i]   = NULL;
        pollfds[i].fd = 0;
        pollfds[i].events = 0;
        pollfds[i].revents = 0;
    }

    bool allowConnect = false;
    bool displayfds = true;
    unsigned int tempIp = 0;

    while ( !exitMain ) {
        fflush(stderr);
        if ( shrink ) {
            pollfds[0].fd = *srv;
            pollfds[0].events = PR_POLL_READ | PR_POLL_EXCEPT;
            pollfds[0].revents = 0;
            nClients = 1;
            offset = 1;
            shift  = 0;

            for ( i = 1; i < MAXCLIENTS; i++ ) {
                if ( pollfds[i].fd == 0 ) {
                    shift = i + offset;

                    if ( shift >= MAXCLIENTS )
                    { break; }

                    if ( pollfds[shift].fd == 0 ) {
                        while ( pollfds[i + offset].fd == 0 && i + offset < MAXCLIENTS ) {
                            offset++;
                        }
                    }

                    if ( ( shift = i + offset ) < MAXCLIENTS ) {
                        //cout << "Shifting " << shift << " to " << i << endl;
                        pollfds[i].fd = pollfds[shift].fd;
                        //pollfds[i].events  =  pollfds[shift].events;
                        pollfds[i].events  =  PR_POLL_READ | PR_POLL_WRITE | PR_POLL_EXCEPT;
                        pollfds[i].revents = 0;
                        pollfds[shift].fd = 0;
                        pollfds[shift].events  = 0;
                        pollfds[shift].revents = 0;
                        conn[i] = conn[shift];
                        conn[shift] = NULL;
                        nClients++;
                    }
                } else {
                    pollfds[i].events  =  PR_POLL_READ | PR_POLL_WRITE | PR_POLL_EXCEPT;
                    pollfds[i].revents = 0;
                    nClients++;
                }
            }

            fprintf(stderr, "Number of clients = %d \n", nClients); //cout << "Number of clients = " << nClients << endl;
            shrink = false;
            displayfds = true;
        }

        if ( displayfds ) {
            int l = 0;
            fprintf ( stderr, "\n" );
            fprintf(stderr, "____________________________________\n" );
            fprintf ( stderr, "Active fds=%d \n", nClients );

            while  ( l < nClients ) {
                fprintf ( stderr, "%d, ", pollfds[l].fd );
                l++;
            }

            fprintf ( stderr, "\n" );
            fprintf ( stderr, "____________________________________\n" );
            displayfds = false;
        }

        if ( ( retVal = PR_Poll ( pollfds, nClients, 100 ) ) > 0 ) {
            if ( pollfds[0].revents & PR_POLL_READ ) {
                socklen_t addrlen = 0;

                //printf("INFO: Received socket accepting \n");
                if ( ( *client = accept ( *srv, ( sockaddr * ) &clientAddr, &addrlen ) ) ) {
                    //TODO:
                    //allowConnect = false;
                    allowConnect = true;
                    tempIp = PR_ntohl ( clientAddr.sin_addr.s_addr );
#if 0

                    for ( n = 0; n < aclCount; n++ ) {
                        if ( tempIp == aclIp[n] ) {
                            allowConnect = true;
                            break;
                        }
                    }

#endif
                    char clientAddress[256];
                    strcpy ( clientAddress, inet_ntoa ( clientAddr.sin_addr ) );
                    //if ( PR_SUCCESS == PR_NetAddrToString ( &clientAddr , clientAddress, 256 ) ){
                    fprintf ( stderr, "INFO: Connection Received from 0x%x, %s \n", tempIp, clientAddress );

                    //} else {
                    //  fprintf(stderr,"INFO: Connection Received from 0x%x \n",tempIp);
                    //}
                    if ( allowConnect ) {
                        if ( nClients < MAXCLIENTS ) {
                            //if( PR_SetSocketOption(client, &opt) == PR_FAILURE )
                            int flags = fcntl ( *client, F_GETFL );
                            flags |= O_NONBLOCK;

                            if ( fcntl ( *client, F_SETFL, flags ) != 0 ) {
                                fprintf ( stderr, "ERRR: Unable to set socket option \n" );
                            }

                            fprintf ( stderr, "INFO: Received Connection on fd=%d\n", *client );
                            pollfds[nClients].fd = *client;
                            pollfds[nClients].events = PR_POLL_READ | PR_POLL_EXCEPT;
                            pollfds[nClients].revents = 0;
                            conn[nClients] = new Connection;
                            * ( conn[nClients]->socket ) = *client;
                            conn[nClients]->ip     = PR_ntohl ( clientAddr.sin_addr.s_addr );
                            conn[nClients]->setAuthLvl();
                            nClients++;
                        } else {
                            fprintf ( stderr, "INFO: Connection closed due to max clients exceeded 0x%x \n", tempIp );
                            PR_Close ( client );
                        }
                    } else {
                        fprintf ( stderr, "WARN: Connection closed because of ACL list entry not present 0x%x \n", tempIp );
                        PR_Shutdown ( client, PR_SHUTDOWN_BOTH );
                        PR_Close ( client );
                    }
                }

                //printf("INFO: Received socket accept finished \n");
            } else {
                if ( pollfds[0].revents & PR_POLL_NVAL || pollfds[0].revents & PR_POLL_ERR ) {
                    fprintf(stderr, "ERRR: Receive socket invalid !!!!!!!!!!! \n" );
                    PR_Closefd ( pollfds[0].fd );
                    pollfds[0].fd = 0;
                    goto start;
                    //exit(1);
                }
            }

            for ( i = 1; i < nClients; i++ ) {


                if ( pollfds[i].revents & PR_POLL_NVAL || pollfds[i].revents & PR_POLL_ERR ) {
                    fprintf(stderr, "INFO: Invalid fd , closing \n" );
                    PR_Shutdownfd ( pollfds[i].fd, PR_SHUTDOWN_BOTH );
                    PR_Closefd ( pollfds[i].fd );
                    if( conn[i] )
                        delete conn[i];
                    conn[i] = 0;
                    pollfds[i].fd = 0;
                    pollfds[i].events = 0;
                    pollfds[i].revents = 0;
                    shrink = true;
                    continue;
                }

                if ( !conn[i] ) {
                    cout << "ERRR: Unable to locate connection data  " << endl;
                    PR_Shutdownfd ( pollfds[i].fd, PR_SHUTDOWN_BOTH );
                    PR_Closefd ( pollfds[i].fd );
                    pollfds[i].fd = 0;
                    pollfds[i].events = 0;
                    pollfds[i].revents = 0;
                    shrink = true;
                    continue;
                }

                if ( pollfds[i].fd > *srv && pollfds[i].revents & PR_POLL_READ ) {
                    HttpReq *temp = &conn[i]->req;
                    int tempLen = 0;

                    if ( temp->hLen <= 0 )
                    { tempLen   = PR_Recvfd ( pollfds[i].fd, &temp->buf[temp->len], MAXBUF - temp->len, 0, 1 ); }
                    else
                    { tempLen   = PR_Recvfd ( pollfds[i].fd, &temp->buf[temp->hLen], MAXBUF - temp->hLen, 0, 1 ); }

                    if ( tempLen > 0 ) {
                        temp->len += tempLen; // Total Data Length;

                        if ( temp->hLen <= 0 ) {
                            temp->readHttpHeader();

                            if ( temp->hLen <= 0 ) {
                                fprintf ( stderr, "WARN: Incomplete header received\n" );
                                //pollfds[i].revents = 0;
                                pollfds[i].revents = pollfds[i].revents & ~PR_POLL_READ;
                                continue;
                            } else {
                                if ( temp->getMethod() == HTTP_POST ) {
                                    temp->processHttpPostData ( temp->hLen, temp->len - temp->hLen );

                                    if (  temp->cLen > temp->len - temp->hLen ) {
                                        //pollfds[i].revents = 0;
                                        pollfds[i].revents = pollfds[i].revents & ~PR_POLL_READ;
                                        continue;
                                    }
                                }
                            }
                        } else {
                            if ( temp->getMethod() == HTTP_POST ) {
                                if ( temp->cLen > 0 ) {
                                    temp->processHttpPostData ( temp->hLen, tempLen );
                                }

                                if ( temp->cLen > temp->len - temp->hLen ) {
                                    //pollfds[i].revents = 0;
                                    pollfds[i].revents = pollfds[i].revents & ~PR_POLL_READ;
                                    continue;
                                }
                            }
                        }

                        MapStrStr *cookies = HttpSession::readCookies ( ( char * ) temp->getCookie() );
                        conn[i]->sess = NULL;

                        if ( cookies ) {
                            MapStrStr::iterator itr = cookies->find ( "SID" );

                            if ( itr != cookies->end() ) {
                                conn[i]->sess = hSMgr->getSession ( conn[i]->ip, itr->second );

                                if ( conn[i]->sess )
                                { fprintf ( stderr, "INFO: Session Active, SID: %s\n", conn[i]->sess->sid.c_str() ); }
                            }

                            delete cookies;
                        }

                        if ( !conn[i]->sess ) {
                            fprintf ( stderr, "WARN: No valid Session Present, creating new \n" );
                            time_t now = time ( NULL );
                            now += 86400;
                            conn[i]->sess = hSMgr->startSession ( conn[i]->ip, now );
                            //TODO: We don't need to save the entire map everytime a new connection comes in
                            //hSMgr->saveSession();
                        }

                        if ( conn[i]->sess ) {
                            //TODO:
                            PRStatus fInfoStatus;
                            bool isForbidden = false;

                            char *fileType = temp->getReqFileType();

                            if ( ( strcmp ( fileType, ".html" ) == 0 ) ||
                                    ( strcmp ( fileType, ".js" )   == 0 ) ||
                                    ( strcmp ( fileType, ".htm" )  == 0 ) ||
                                    ( strcmp ( fileType, ".xyz" )  == 0 ) ||
                                    ( strcmp ( fileType, ".css" )  == 0 )
                               ) {
                                fprintf ( stderr, "WARN: Requesting html/scriptfile file type \n" );
                                char *authFile = temp->getReqFileAuth ( conn[i]->authLvl );
                                * ( conn[i]->file )  = PR_Open ( authFile, PR_RDONLY, 0 );
                                fInfoStatus    = PR_GetFileInfo64 ( * ( conn[i]->file ), & ( conn[i]->fInfo ) );
                            } else if ( ( strcmp ( fileType, ".png" )   == 0 ) ||
                                        ( strcmp ( fileType, ".jpg" )   == 0 ) ||
                                        ( strcmp ( fileType, ".ico" )   == 0 ) ||
                                        ( strcmp ( fileType, ".bmp" )   == 0 ) ||
                                        ( strcmp ( fileType, ".jpeg" )  == 0 )
                                      ) {
                                fprintf ( stderr, "WARN: Requesting image file type \n" );
                                * ( conn[i]->file ) = PR_Open ( temp->getReqFile(), PR_RDONLY, 0 );
                                fInfoStatus   = PR_GetFileInfo64 ( * ( conn[i]->file ), & ( conn[i]->fInfo ) );
                            } else {
                                isForbidden = true;
                                fprintf ( stderr, "WARN: Requesting unsupported file type \n" );
                                HttpResp *tempResp   = &conn[i]->resp;
                                //tempResp->setContentLen( conn[i]->fid->fileSize);
                                tempResp->setStatus ( 403 );
                                tempResp->setContentLen ( 0 );
                                tempResp->setAddOn ( 1 );
                                pollfds[i].events  = PR_POLL_READ | PR_POLL_EXCEPT | PR_POLL_WRITE;
                                tempResp->setContentType ( ( char * ) identifyContentType ( temp->getReqFile() ) );
                                conn[i]->len = tempResp->getHeader ( ( char * ) conn[i]->buf );
                                conn[i]->len += conn[i]->sess->dumpSessionCookies ( ( char * ) & ( conn[i]->buf[conn[i]->len] ) );
                                conn[i]->len += tempResp->finishHdr ( ( char * ) & ( conn[i]->buf[conn[i]->len] ) );
                                fprintf ( stderr, "\nDBUG: Response Header: \n%s\n", ( char * ) conn[i]->buf );
                                fprintf ( stderr, "DBUG: ____________________________________\n" );
                            }

                            /*
                             * Validate Authorization
                             */
                            if ( conn[i]->file && fInfoStatus == PR_SUCCESS ) {
                                //TODO:
                                HttpResp *tempResp   = &conn[i]->resp;
                                //tempResp->setContentLen( conn[i]->fid->fileSize);
                                tempResp->setContentLen ( conn[i]->fInfo.st_size );
                                tempResp->setAddOn ( 1 );
                                pollfds[i].events  = PR_POLL_READ | PR_POLL_EXCEPT | PR_POLL_WRITE;
                                tempResp->setContentType ( ( char * ) identifyContentType ( temp->getReqFile() ) );
                                conn[i]->len = tempResp->getHeader ( ( char * ) conn[i]->buf );
                                conn[i]->len += conn[i]->sess->dumpSessionCookies ( ( char * ) & ( conn[i]->buf[conn[i]->len] ) );
                                conn[i]->len += tempResp->finishHdr ( ( char * ) & ( conn[i]->buf[conn[i]->len] ) );
                                fprintf ( stderr, "\nDBUG: Response Header: \n%s\n", ( char * ) conn[i]->buf );
                                fprintf ( stderr, "DBUG: ____________________________________\n" );
                            } else {
                                if ( ! isForbidden ) {
                                    conn[i]->cmd  = THREAD_CMD_PTASK;
                                    tMgr->assignTask ( conn[i] );
                                    conn[i]       = NULL;
                                    pollfds[i].fd = 0;
                                    pollfds[i].events = 0;
                                    pollfds[i].revents = 0;
                                    shrink = true;
                                }
                            }
                        }
                    } else {
                        //char ERRRSTR[256];
                        //PR_GetErrorText ( ERRRSTR );
                        //cout << " Client Disconnected :" << tempLen << ", PRError: " << ERRRSTR << endl;
                        PR_Shutdownfd ( pollfds[i].fd, PR_SHUTDOWN_BOTH );
			if( conn[i] )
                            delete conn[i];
                        conn[i] = 0;
                        pollfds[i].fd = 0;
                        pollfds[i].events = 0;
                        pollfds[i].revents = 0;
                        shrink = true;
                    }

                    //pollfds[i].revents = 0;
                    pollfds[i].revents = pollfds[i].revents & ~PR_POLL_READ;
                }

                if ( pollfds[i].fd > *srv && pollfds[i].revents & PR_POLL_WRITE && * ( conn[i]->file ) > *srv ) {
                    int len = conn[i]->len;

                    //printf("DBUG: reading file %s \n", conn[i]->file );
                    if ( len < SMALLBUF && conn[i]->file ) {
                        //read in some more data
                        //int temp = ifs->ifsRead( conn[i]->file, conn[i]->fid, &(conn[i]->buf[len]), SMALLBUF-len, (int *)&(conn[i]->offset) );
                        int temp = PR_Read ( conn[i]->file, & ( conn[i]->buf[len] ), SMALLBUF - len );

                        //TODO: use fread
                        if ( temp <= 0 ) {
                            PR_Close ( conn[i]->file );
                            conn[i]->file = NULL;
                        } else {
                            len += temp;
                            conn[i]->len = len;
                        }
                    }

                    //printf("DBUG: reading file done\n");

                    if ( len > 0 ) {
                        int bytesW = 0;

                        do {
                            int temp = PR_Write ( conn[i]->socket, conn[i]->buf, conn[i]->len );

                            if ( temp >= 0 )
                            { bytesW += temp; }
                            else {
                                PRErrorCode error = PR_GetError();

                                if ( error == PR_WOULD_BLOCK_ERROR ) {
                                    memcpy ( conn[i]->buf, & ( conn[i]->buf[bytesW] ), len - bytesW );
                                    break;
                                } else {
                                    bytesW = -999;
                                    break;
                                }
                            }
                        } while ( bytesW < len );

                        if ( bytesW != -999 ) {
                            conn[i]->len   = len - bytesW;
                            conn[i]->sent += bytesW;
                        } else {
                            cout << "ERRR: Socket Write Error " << endl;
                            PR_Shutdownfd ( pollfds[i].fd, PR_SHUTDOWN_BOTH );
                            if( conn[i] )
                                delete conn[i];
                            pollfds[i].fd = 0;
                            pollfds[i].events = 0;
                            pollfds[i].revents = 0;
                            shrink = true;
                        }
                    } else {
                        fprintf(stderr, "\nINFO: --------------------------------------------------------------------------\n" );
                        fprintf ( stderr, "INFO: Sent File='%s' Total bytes=%d (including header)\n", conn[i]->req.getReqFile(), conn[i]->sent );
                        fprintf(stderr, "DBUG: Cleaning up fd=%d \n", pollfds[i].fd );
                        fprintf ( stderr, "INFO: --------------------------------------------------------------------------\n\n\n" );
                        PR_Shutdownfd ( pollfds[i].fd, PR_SHUTDOWN_BOTH );
                        if( conn[i] )
                            delete conn[i];
                        pollfds[i].fd = 0;
                        fprintf(stderr, "DBUG: Cleaning up fd=%d \n", pollfds[i].fd );
                        pollfds[i].events = 0;
                        pollfds[i].revents = 0;
                        shrink = true;
                    }
                }


                pollfds[i].revents = 0;
            }
        } else {
            if ( retVal < 0 ) {
                PR_Sleep ( 1 );
                fprintf ( stderr, "ERRR: Poll failed \n" );
            }
        }
    }

    PR_Shutdown ( srv, PR_SHUTDOWN_BOTH );
    PR_Close ( srv );
    PR_Cleanup();
}

