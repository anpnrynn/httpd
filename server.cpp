//Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3
#include <iostream>
#include <atomic>
#include <exception>
#include <map>
#include <prwrapper.h>
#include <sqlite3.h>
#include <httpconn.h>
#include <version.h>
#include <httpcodes.h>
#include <mimetypes.h>
#include <session.h>
#include <threadmgr.h>
#include <plugin.h>
#include <defines.h>
#include <malloc.h>

#include <sys/time.h>
#include <sys/resource.h>

#undef USE_MINIUPNP
#ifdef USE_MINIUPNP
#include <upnp.h>
#endif


#ifndef USE_CPP11THREAD
#include <pthread.h>
#endif


#ifdef LINUX_BUILD
#include <signal.h>
#endif

#include <thread>
#include <chrono>

#include <log.h>
#define USE_SSL 1


using namespace std;
PRLibrary *lib[MAXPLUGINS];


Connection   **global_conn;
PRPollDesc   **global_pollfds;
int            global_MAXCLIENTS;

void memoryIssue ( int s ) {
    int mi = 0;
    fflush(stdout);
    sleep(10);
    Connection **conn = global_conn;

    while ( mi < global_MAXCLIENTS && conn[mi] ) {
        debuglog ( " crash and burn on deleted %llu -> conn[%d] = %lld \n",
                   conn[mi], mi, conn[mi]->index );
        mi++;
    }

    exit ( 100 );
}

//int srvSocket = 0;
//int clientSocket = 0;
//int *srv = &srvSocket, *client = &clientSocket;
int srv = 0, client = 0;

//int srvSocket6 = 0;
//int clientSocket6 = 0;
//PRFileDesc *srv6 = &srvSocket6, *client6 = &clientSocket6;
int srv6 = 0, client6 = 0;

#ifdef USE_SSL
//int sslsrvSocket = 0;
//int sslclientSocket = 0;
int sslsrv =0, sslclient = 0;

int sslsrvSocket6 = 0;
int sslclientSocket6 = 0;
int sslsrv6 = 0, sslclient6 = 0;
#endif

unsigned short int SRVPORT = 0;
unsigned short int SRVPORT6 = 0;
unsigned short int SSLSRVPORT = 0;
unsigned short int SSLSRVPORT6 = 0;
bool isBound     = false;
bool isBound6    = false;
bool isBoundSsl  = false;
bool isBoundSsl6 = false;
extern int MAX_THREADS;

std::atomic<int> assignCounter(0);
std::atomic<int> createCounter(0);
std::atomic<int> deleteCounter(0);

//Clean up operations
void performCleanupOperations() {

    ThreadMgr *tMgr = 0;

    if ( MAX_THREADS > 0 ) {
        tMgr = ThreadMgr::createInstance();

        if ( isBound && tMgr )
        { tMgr->stopThreads(); }
    } else {
        debuglog (  "INFO: MAX_THREADS == 0, no threads present to stop \n" );
    }
}

void signalIgnore ( int signal ) {
}

//SIGNAL HANDLER
void signalStop ( int signal ) {
    debuglog (  "WARN: Received signal to terminate from user : %d \n", signal );

    if ( srv )
    { close ( srv ); }
    if ( srv6 )
    { close ( srv6 ); }
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

MapACL aclMap;
MapACL dosMap;
std::mutex dosMapMutex;

int aclLoad ( void *udata, int argc, char **argv, char **col ) {
    if ( argv[0] && argv[1] && argv[2] && argv[3] && argv[4] && argv[5] && argv[6] ) {
        Acl *acl = new Acl;
        acl->invert = atoi ( argv[1] );
        acl->ipv4   = atoi ( argv[2] );
        acl->subnetmask = atoi ( argv[3] );
        acl->prefixmask = atoi ( argv[4] );
        acl->ip   = argv[5];
        acl->epochtime = atoll ( argv[6] );

        if ( !acl->invert ) {
            aclMap[acl->ip] = acl;
        }

        acl->counter = 0;
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
            debuglog (  "WARN: Unable to load library:%s \n", lName );
            sprintf ( lName, "/usr/local/lib/lib%s.so", argv[0] );
            lib[libCount] = PR_LoadLibrary ( lName );

            if ( !lib[libCount] ) {
                debuglog (  "WARN: Unable to load library:%s \n", lName );
                sprintf ( lName, INSTALL_HOME "/httpd/lib/lib%s.so", argv[0] );
                lib[libCount] = PR_LoadLibrary ( lName );

                if ( !lib[libCount] ) {
                    debuglog (  "ERRR: Library:%s NOT FOUND\n", lName );
                    lib[libCount] = NULL;
                }
            }
        }

        if ( lib[libCount] ) {
            debuglog (  "WARN: Loaded Library:%s \n", lName );
            libCount++;
        }

#else
        sprintf ( lName, "%s.dll", argv[0] );
        lib[libCount] = PR_LoadLibrary ( lName );

        if ( !lib[libCount] ) {
            debuglog (  "ERRR: Library: %s NOT FOUND\n", lName );
        } else {
            debuglog (  "WARN: Loaded Library: %s\n", lName );
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
        debuglog (  "ERRR: Unable to open db, Corrupted ? \n" );
        sqlite3_close ( db );
        db = NULL;
        return 1;
    }

    rc = sqlite3_exec ( db, "select name,active from plugins;", pluginLoader, NULL, &error );

    if ( rc != SQLITE_OK ) {
        debuglog (  "ERRR: Unable to load necessary plugins \n" );
        sqlite3_close ( db );
        db = NULL;
        return 2;
    }

    rc = sqlite3_exec ( db, "select * from acl;", aclLoad, NULL, &error );

    if ( rc != SQLITE_OK ) {
        debuglog (  "ERRR: Unable to load acl details \n" );
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
                //  debuglog ( "INFO: Plugin Initialized \n");
                //}
            } else {
                debuglog (  "ERRR: Unable to locate symbol in object code \n" );
            }
        } else {
            debuglog (  "ERRR: Plugin Data Structure Empty \n" );
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
        debuglog (  "ERRR: Unable to open db, Corrupted ? %s \n", INFO_STORE );
        sqlite3_close ( db );
        db = NULL;
        return 2;
    } else {
        debuglog (  "DBUG: able to open db: %s \n", INFO_STORE );
    }

    //int filefd = 0;
    FILE *file = 0;
    debuglog (  "INFO: reading sql tables from : %s \n", INSTALL_HOME "/share/install.sql" );
#ifndef CODEBLOCKS_BUILD
    file = fopen ( INSTALL_HOME "/share/install.sql", "r+" );
#else
    file = fopen ( INSTALL_HOME "/share/install.sql", "r+" );
#endif


    //if( !file || !fData )
    if ( !file  ) {
        debuglog (  "ERRR: Unable to open install.sql file \n" );
        return 3;
    }

    int i = 0, j = 0, bRead = 0;

    rc = sqlexecute ( db, "BEGIN IMMEDIATE;", NULL, NULL, NULL );

    //while ( ( bRead = ifs->ifsRead( file, fData, (unsigned char *)&buf, 256, &offset ) ) > 0 )
    while ( ( bRead = fread ( ( unsigned char * ) &buf, 1, 256, file ) ) > 0 ) {
        i = 0;
        debuglog("DBUG: Read %d bytes from install.sql \n", bRead);
        while ( i < bRead ) {
            if ( buf[i] == '\r' )
            { cmd[j] = 0;
                debuglog("DBUG: Encountered cr in the data \n");
            }
            else if ( buf[i] == '\n' ) {
                    debuglog("DBUG: Encountered lf in the data \n");
                cmd[j] = 0;
                error  = NULL;
                debuglog (  "DBUG: Running Cmd '%s' \n", cmd );
                rc = sqlite3_exec ( db, cmd, NULL, NULL, &error );

                if ( rc != SQLITE_OK ) {
                    if ( error ) {
                        sqlite3_free ( error );
                    }

                    debuglog (  "DBUG: Unable to run command '%s'\n", cmd );
                    sqlite3_close ( db );
                    fclose ( file );
                    return 4;
                }

                j = 0;
            } else
            { cmd[j++] = buf[i]; }

            i++;
        }
    }
    debuglog (  "DBUG: Commiting changes \n" );
    rc = sqlexecute ( db, "COMMIT;", NULL, NULL, NULL );
    sqlite3_close ( db );
    fclose ( file );
    debuglog (  "INFO: Done installing prerequisites \n" );
//#endif
    return 0;
}

#define MAX_BACKLOG 1024

atomic_int clientsOnboard;
int clientmanage ( int op ) {
    switch ( op ) {
        case 0: {
                if ( clientsOnboard > 0 ) //remove
                { clientsOnboard--; }
            }
            break;

        case 1: {
                if ( clientsOnboard < MAX_BACKLOG ) //remove
                { clientsOnboard++; }
                else
                { debuglog (  "WARN: Too many BACKLOGS, Not accepting any new connections \n" ); }
            }
            break;

        case 2:
            return clientsOnboard;
    };

    return -1;
}

void reduce_r ( string ip ) {
    dosMapMutex.lock();
    MapACL::iterator iter = dosMap.find ( ip );

    if ( iter != dosMap.end() ) {
        debuglog ( " DOS PREVENTION: Reducing counter for : %s -> %d \n", ip.c_str(), iter->second->counter ) ;
        iter->second->counter -= 1;

        if ( iter->second->counter < 0 ) {
            iter->second->counter = 0;
        }
    }

    dosMapMutex.unlock();
}

void reduce_ipbin ( Connection *conn ) {
    char clientAddress[64];

    if ( conn->ip == 0 ) {
        inet_ntop ( AF_INET6, & ( conn->ipv6[0] ), clientAddress, 64 );
    } else {
        sockaddr_in addr;
        addr.sin_addr.s_addr = conn->ip;
        inet_ntop ( AF_INET, & addr.sin_addr,  clientAddress, 64 );
    }

    string ipstr = clientAddress;
    reduce_r ( ipstr );
}

void reverse ( unsigned char *data, int len ) {
    int i = 0;
    int n = len / 2;

    while ( i < n ) {
        unsigned char c = data[i];
        data[i] = data [n - i - 1];
        data[n - i - 1] = c;
        i++;
    }
}


void safe_delete( Connection *curconn ){
	typedef std::map<void*, int> freedmemory;
	static freedmemory fm;
	if( fm.find(curconn) == fm.end() ){
		fm.insert(std::pair<void*,int>(curconn, 0));
		debuglog("ERRR: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ Deleting now  0x%X \n", curconn );
		delete curconn;
	} else {
		debuglog("ERRR: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ Already deleted 0x%X \n", curconn );
	}
}

void conn_close ( Connection *curconn ) {
    if ( curconn ) {
#ifdef USE_SSL

        if ( curconn->ssl ) {
            SSL_shutdown ( curconn->ssl );
            SSL_free ( curconn->ssl );
            curconn->ssl = 0;
            curconn->is_ssl = false;
            curconn->ssl_accept = false;
        }

#endif
        if( curconn->file != 0 ){
            fclose( curconn->file );
        }
        reduce_ipbin ( curconn );
        curconn->delobj = true;
        clientmanage ( 0 );
        debuglog("ERRR: ***************************** Closing & deleting connection, 0x%X, %d \n",curconn, curconn->index);
        //safe_delete( curconn );
	delete curconn;
        deleteCounter++;
    } else {
        debuglog("ERRR: conn_close called on null object \n");
    }
}



int main ( int argc, char *argv[] ) {
    bool  isRestart = false;
    short firstTime = 0;
    clientsOnboard  = 0;


    loglevel = 1;

    //fclose ( stderr );
    //stderr = NULL;
    //stderr = fopen ( LOGFILE, "a+" );
    cerr << "HTTPD " << VERSION << " Server starting up... " << endl;

#ifdef USEMALLOC_CHECK
    mallopt ( M_CHECK_ACTION, 0x05 );
#endif

    if ( !stderr )
    { exit ( 56 ); }

    if ( argc < 6 ) {
        debuglog (  " %s Exited \n", argv[0] );
        debuglog (  " Format: %s port count dosthreshold sslport loglevel ipv6\n", argv[0] );
        debuglog (  " port    - Server port number to use \n" );
        debuglog (  " count   - Number of threads to launch \n" );
        debuglog (  " dosthrehsold   - DOS threshold \n" );
        debuglog (  " sslport - SSL server port number to use \n" );
        debuglog (  " loglevel - 1-6, 6 churning out most logs \n" );
        debuglog (  " ipv6 address to bind to \n" );
        exit ( 1 );
    } else {
        SRVPORT = atoi ( argv[1] );
        SRVPORT6 = atoi ( argv[1] );
        SSLSRVPORT = atoi ( argv[4] );
        SSLSRVPORT6 = atoi ( argv[4] );
        loglevel = atoi ( argv[5] );
    }

    int DOS_THRESHOLD = 50;

    if ( argc >= 3 ) {
        debuglog (  " Received threads count: %s \n", argv[2] );
        MAX_THREADS = atoi ( argv[2] );
        DOS_THRESHOLD = atoi ( argv[3] );

        if ( MAX_THREADS > MAX_POSSIBLE_THREADS ) {
            debuglog (  "ERRR: Cannot create more than %d threads \n", MAX_POSSIBLE_THREADS );
            exit ( 1 );
        } else if ( MAX_THREADS == 0 ) {
            debuglog (  "WARN: MAX_THREADS == 0, disabling threadmgr and dynamic pages \n" );
        } else {
            if ( MAX_THREADS < 0 ) {
                debuglog (  "ERRR: Number of threads less than zero \n" );
                exit ( 1 );
            }
        }
    }

    debuglog (  "INFO: Using db : %s  \n", INFO_STORE );

#ifdef LINUX_BUILD
    signal ( SIGINT, signalStop );
    signal ( SIGSTOP, signalStop );
    signal ( SIGABRT, signalStop );
#ifdef  MINGW64_BUILD
    signal ( SIGPIPE, signalIgnore );
#else
    struct sigaction sh;
    struct sigaction osh;
    sh.sa_handler = &signalIgnore;
    sh.sa_flags = SA_RESTART;
    sigemptyset ( &sh.sa_mask );

    if ( sigaction ( SIGPIPE, &sh, &osh ) < 0 ) {
        return -1;
    }

    sigset_t bs;
    sigemptyset ( &bs );
    sigaddset ( &bs, SIGPIPE );

    pthread_sigmask ( SIG_BLOCK, &bs, NULL );
#endif
#endif

start:
    sockaddr_in  srvAddr;
    sockaddr_in  clientAddr;

    sockaddr_in6 srvAddr6;
    sockaddr_in6 clientAddr6;

#ifdef USE_SSL
    sockaddr_in  sslsrvAddr;
    sockaddr_in  sslclientAddr;

    sockaddr_in6 sslsrvAddr6;
    sockaddr_in6 sslclientAddr6;
#endif

    debuglog (  "INFO: Creating socket(s) \n" );
    srv = socket(PF_INET,  SOCK_STREAM, IPPROTO_TCP);
    srvAddr.sin_family  = AF_INET;
    srvAddr.sin_addr.s_addr     = 0x00000000;
    srvAddr.sin_port   = htons ( SRVPORT );
    debuglog (  "INFO: IPv4 Socket created successfully : Port = %d \n", SRVPORT );

    srv6 = socket(PF_INET6,  SOCK_STREAM, IPPROTO_TCP);
    memset(&srvAddr6, 0, sizeof(struct sockaddr_in6));
    srvAddr6.sin6_family = AF_INET6;
    SRVPORT6 = SRVPORT;
    if( argc >= 6 && argv[6] )
    {
	    debuglog("WARN: using %s address \n",argv[6] );
	    inet_pton ( AF_INET6, argv[6], &(srvAddr6.sin6_addr) );
    }
    else
	    srvAddr6.sin6_addr = in6addr_loopback;

    srvAddr6.sin6_port = htons ( SRVPORT6 );
    debuglog (  "INFO: IPv6 Socket created successfully : Port = %d \n", SRVPORT6 );

#ifdef USE_SSL
    debuglog (  "INFO: Creating ssl socket(s) \n" );
    sslsrv = socket(PF_INET,  SOCK_STREAM, IPPROTO_TCP);
    sslsrvAddr.sin_family  = AF_INET;
    sslsrvAddr.sin_addr.s_addr     = 0x00000000;
    sslsrvAddr.sin_port   = htons ( SSLSRVPORT );
    debuglog (  "INFO: SSL IPv4 Socket created successfully : Port = %d \n", SSLSRVPORT );

    sslsrv6 = socket(PF_INET6,  SOCK_STREAM, IPPROTO_TCP);
    memset(&sslsrvAddr6, 0, sizeof(struct sockaddr_in6));
    sslsrvAddr6.sin6_family = AF_INET6;
    SSLSRVPORT6 = SSLSRVPORT;
    if( argc >= 6 && argv[6] ){
        debuglog("WARN: using %s address for ssl \n",argv[6] );
	    inet_pton ( AF_INET6, argv[6], &(sslsrvAddr6.sin6_addr) );
    }else
	    sslsrvAddr6.sin6_addr = in6addr_loopback;
    sslsrvAddr6.sin6_port = htons ( SSLSRVPORT6 );
    debuglog (  "INFO: SSL IPv6 Socket created successfully : Port = %d \n", SSLSRVPORT6 );
#endif


    int sockflag = 1;
    int sockret = setsockopt ( srv, SOL_SOCKET, SO_REUSEADDR , (char *)&sockflag, sizeof ( sockflag ) );

    if ( sockret == -1 ) {
        debuglog (  "ERRR: Unable to setsockopt - IPv4 \n" );
        return 1;
    }

    sockflag = 1;
    sockret = setsockopt ( srv6, SOL_SOCKET, SO_REUSEADDR , (char *)&sockflag, sizeof ( sockflag ) );

    if ( sockret == -1 ) {
        debuglog (  "ERRR: Unable to setsockopt - IPv6 \n" );
        return 1;
    }

#ifdef USE_SSL
    sockflag = 1;
    sockret = setsockopt ( sslsrv, SOL_SOCKET, SO_REUSEADDR , (char *)&sockflag, sizeof ( sockflag ) );

    if ( sockret == -1 ) {
        debuglog (  "ERRR: Unable to setsockopt - SSLIPv4 \n" );
        return 1;
    }

    sockflag = 1;
    sockret = setsockopt ( sslsrv6, SOL_SOCKET, SO_REUSEADDR , (char *)&sockflag, sizeof ( sockflag ) );

    if ( sockret == -1 ) {
        debuglog (  "ERRR: Unable to setsockopt - SSLIPv6 \n" );
        return 1;
    }

#endif


    int count = 0;
    isBound = true;
    while ( bind ( srv, ( const sockaddr * ) &srvAddr, sizeof ( sockaddr_in ) ) == -1 ) {
            perror ( "bind" );
        debuglog (  "ERRR: Unable to Bind - IPv4 \n" );
        perror ( "bind" );
        std::this_thread::sleep_for ( std::chrono::microseconds ( 100000 ) );

        if ( count > 10 ) {
            //return 1;
            isBound = false;
            break;
        } else {
            count++;
        }
    }

    count = 0;


    while ( bind ( srv6, ( const sockaddr * ) &srvAddr6, sizeof ( sockaddr_in6 ) ) !=  0 ) {
	char address[64];
	inet_ntop( AF_INET6, &(srvAddr6.sin6_addr), address , 64 );
        debuglog (  "ERRR: Unable to Bind - IPv6 , %s %d\n", address, errno );
        perror ( "bind" );
        std::this_thread::sleep_for ( std::chrono::microseconds ( 100000 ) );

        if ( count > 10 ) {
            isBound6 = false;
            break;
        } else {
            count++;
        }
    }

    isBound = true;
    isBound6 = true;

#ifdef USE_SSL

    isBoundSsl = true;
    while ( bind ( sslsrv, ( const sockaddr * ) &sslsrvAddr, sizeof ( sockaddr_in ) ) == -1 ) {
        debuglog (  "ERRR: Unable to Bind - SSL IPv4 \n" );
        perror ( "bind" );
        std::this_thread::sleep_for ( std::chrono::microseconds ( 100000 ) );

        if ( count > 10 ) {
            isBoundSsl = false;
            break;
        } else {
            count++;
        }
    }

    count = 0;

    isBoundSsl6 = true;
    while ( bind ( sslsrv6, ( const sockaddr * ) &sslsrvAddr6, sizeof ( sockaddr_in6 ) ) == -1 ) {
        debuglog (  "ERRR: Unable to Bind - SSL IPv6 \n" );
        perror ( "bind" );
        std::this_thread::sleep_for ( std::chrono::microseconds ( 100000 ) );

        if ( count > 10 ) {
            isBoundSsl6 = true;
            break;
        } else {
            count++;
        }
    }

    isBoundSsl = true;
    isBoundSsl6 = true;
#endif

    debuglog (  "INFO: Bound successfully both IPv4 and IPv6 \n" );

    if ( listen ( srv, 20 ) != 0 ) {
        debuglog (  "ERRR: Unable to setup backlog - IPV4\n" );
        PR_Cleanup();
        return 2;
    }

    if ( listen ( srv6, 20 ) != 0 ) {
        debuglog (  "ERRR: Unable to setup backlog - IPv6\n" );
        PR_Cleanup();
        return 2;
    }

#ifdef USE_SSL

    if ( listen ( sslsrv, 20 ) != 0 ) {
        debuglog (  "ERRR: Unable to setup backlog - SSL IPV4\n" );
        PR_Cleanup();
        return 2;
    }

    if ( listen ( sslsrv6, 20 ) != 0 ) {
        debuglog (  "ERRR: Unable to setup backlog - SSL IPv6\n" );
        PR_Cleanup();
        return 2;
    }

#endif

    debuglog (  "INFO: Listening successfully IPv4 & IPv6 \n" );
    //PRSocketOptionData  opt;
    //opt.option = PR_SockOpt_Nonblocking;
    //opt.value.non_blocking = true;
    u_long mode = 1; // 1 to enable non-blocking mode
#if 0
    ioctlsocket(srv, FIONBIO, &mode);
    ioctlsocket(srv6, FIONBIO, &mode);
    ioctlsocket(sslsrv, FIONBIO, &mode);
    ioctlsocket(sslsrv6, FIONBIO, &mode);
#endif

    //if( PR_SetSocketOption(srv, &opt) == PR_FAILURE )
    if ( fcntl ( srv, F_SETFL, O_NONBLOCK ) != 0 ) {
        debuglog (  "ERRR: Unable to set fd in NONBLOCK  - IPv4 \n" );
        PR_Cleanup();
        return 3;
    }

    if ( fcntl ( srv6, F_SETFL, O_NONBLOCK ) != 0 ) {
        debuglog (  "ERRR: Unable to set fd in NONBLOCK  - IPv6 \n" );
        PR_Cleanup();
        return 3;
    }

#ifdef USE_SSL

    if ( fcntl ( sslsrv, F_SETFL, O_NONBLOCK ) != 0 ) {
        debuglog (  "ERRR: Unable to set fd in NONBLOCK  - IPv4 \n" );
        PR_Cleanup();
        return 3;
    }

    if ( fcntl ( sslsrv6, F_SETFL, O_NONBLOCK ) != 0 ) {
        debuglog (  "ERRR: Unable to set fd in NONBLOCK  - IPv6 \n" );
        PR_Cleanup();
        return 3;
    }
#endif

    debuglog (  "INFO: Non blocking successfully - IPv4 & IPv6 \n" );
    fflush ( stdout );


#ifdef USE_SSL
#define SSL_PORT_COUNT 2
    const SSL_METHOD *mIp4;
    SSL_CTX *cIp4;

    mIp4 = TLS_server_method();

    cIp4 = SSL_CTX_new ( mIp4 );

    if ( !cIp4 ) {
        debuglog (  "ERRR: Unable to create SSL context" );
        return ( 1000 );
    }

    if ( SSL_CTX_use_certificate_file ( cIp4, CERT_STORE "httpdcert.pem", SSL_FILETYPE_PEM ) <= 0 ) {
        debuglog (  "ERRR: Certificate file issue\n" );
        return ( 1001 );
    } else {
        debuglog (  "INFO: Certiticate file loaded : %s \n", CERT_STORE "httpdcert.pem" );
    }

    if ( SSL_CTX_use_PrivateKey_file ( cIp4, CERT_STORE "httpdkey.pem", SSL_FILETYPE_PEM ) <= 0 ) {
        debuglog (  "ERRR: Private key file issue\n" );
        return ( 1002 );
    } else {
        debuglog (  "INFO: Private key file loaded : %s \n", CERT_STORE "httpdkey.pem" );
    }

    const SSL_METHOD *mIp6;
    SSL_CTX *cIp6;

    mIp6 = TLS_server_method();

    cIp6 = SSL_CTX_new ( mIp6 );

    if ( !cIp6 ) {
        debuglog (  "ERRR: Unable to create SSL context" );
        return ( 1000 );
    }

    if ( SSL_CTX_use_certificate_file ( cIp6, CERT_STORE "httpdcert6.pem", SSL_FILETYPE_PEM ) <= 0 ) {
        debuglog (  "ERRR: Certificate file issue\n" );
        return ( 1001 );
    } else {
        debuglog (  "INFO: Certificate file loaded : %s \n", CERT_STORE "httpdcert6.pem" );
    }

    if ( SSL_CTX_use_PrivateKey_file ( cIp6,  CERT_STORE "httpdkey6.pem", SSL_FILETYPE_PEM ) <= 0 ) {
        debuglog (  "ERRR: Private key file issue\n" );
        return ( 1002 );
    } else {
        debuglog (  "INFO: Private key file loaded : %s \n", CERT_STORE "httpdkey6.pem" );
    }

#else
#define SSL_PORT_COUNT 0
#endif


#if 0

    if ( 0 != acquireEncryptionKeys ( &firstTime ) ) {
        debuglog (  "ERRR: Acquiring Encryption Keys Failed \n" );
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

        //int infostoreFd = 0;
        FILE *infoStore = 0;
        infoStore = fopen ( INFO_STORE, "r+" );

        if ( infoStore  ) {
            debuglog (  "WARN: Database present \n" );
            fclose ( infoStore );

            if ( firstTime ) {
                debuglog (  "WARN: Database key file not present, but database present \n" );
                debuglog (  "WARN: Please backup the database to another location and restart this program \n" );
                //PR_Cleanup();
                return 10;
            }
        } else {
            debuglog (  "WARN: Database not present, installing the needed stuff \n" );

            if ( 0 != installTheNeededStuff() ) {
                debuglog (  "ERRR: Unable to perform internal installation \n" );
                //PR_Cleanup();
                return 11;
            }

        }

        debuglog (  "WARN: Starting session manager \n" );
        hSMgr = HttpSessionMgr::createInstance();
        /*
         *  Restore persistent sessions
         */
#if 1

        if ( hSMgr->loadStoredSessions() != 0 ) {
            debuglog (  "ERRR: Unable to restore session data \n" );
            return 13;
        } else {
            debuglog (  "INFO: Reading stored data completed \n" );
        }

#endif
        debuglog (  "WARN: Clearing plugins \n" );
        clearPlugins();
        loadInfo();
        initPlugins();
        debuglog (  "WARN: Starting plugins \n" );
#ifdef LINUX_BUILD
        //malloc_trim ( 0 );
#endif
    }

    setupStatusCodes();
    setupContentTypes();
    ThreadMgr *tMgr = 0;

    if ( MAX_THREADS > 0 ) {
        tMgr = ThreadMgr::createInstance();

        if ( tMgr )
        { tMgr->startThreads(); }
    }


    //TODO:
    //start upnp and natpmp thread here.

    const int MAXCLIENTS = MAX_THREADS <= 10 ? 1024 : MAX_THREADS * CONST_MAXCLIENTS / 10;
    debuglog (  "MAXCLIENTS supported as of now is %d \n", MAXCLIENTS );

    struct rlimit r;

    if ( getrlimit ( RLIMIT_NOFILE, &r ) == 0 ) {
        debuglog (  "INFO: Current rlimit open fds: %ld %ld \n", r.rlim_cur, r.rlim_max );
        r.rlim_cur = MAXCLIENTS * 4 + 4;
        r.rlim_max = ( MAXCLIENTS * 4 + 4 ) > 1048576 ? MAXCLIENTS * 4 + 4 : r.rlim_max;
    } else {
        r.rlim_cur = MAXCLIENTS * 4 + 4;
        r.rlim_max = 1048576;
    }

    if ( setrlimit ( RLIMIT_NOFILE, &r ) == 0 ) {
        if ( getrlimit ( RLIMIT_NOFILE, &r ) == 0 ) {
            debuglog (  "INFO: Reset to current rlimit open fds : %ld %ld \n", r.rlim_cur, r.rlim_max );
        }
    } else {
        debuglog (  "ERRR: Reset to rlimit failed \n" );
    }

    Connection   **conn = new Connection*[MAXCLIENTS + 2];
    PRPollDesc   *pollfds = new PRPollDesc[MAXCLIENTS + 2];
    global_conn  = conn;
    global_MAXCLIENTS = MAXCLIENTS;
    //Connection **conn = ( Connection ** ) malloc ( MAXCLIENTS * sizeof ( Connection* ) );
    //PRPollDesc *pollfds = ( PRPollDesc* ) malloc ( MAXCLIENTS * sizeof ( PRPollDesc ) );
    //

    signal ( SIGSEGV, memoryIssue );

    bool exitMain = false;
    bool shrink   = true;
    int  nClients = 1;
    int  offset   = 1;
    int  shift    = 0;
    int  retVal   = 0;
    int  i;

    for ( i = 0; i < MAXCLIENTS; i++ ) {
        conn[i]   = 0;
        pollfds[i].fd = 0;
        pollfds[i].events = 0;
        pollfds[i].revents = 0;
    }

    bool allowConnect = false;
    bool displayfds = true;
    unsigned int tempIp = 0;
    cerr << "HTTPD " << VERSION << " Server Up !" << endl;

    while ( !exitMain ) {
        fflush ( stderr );

shrinkStart:

        if ( shrink ) {
#define MIN_SERVER_PORTS 2
            pollfds[0].fd = srv;
            pollfds[0].events = PR_POLL_READ;// | PR_POLL_ERR | PR_POLL_HUP | PR_POLL_NVAL;
            pollfds[0].revents = 0;

            pollfds[1].fd = srv6;
            pollfds[1].events = PR_POLL_READ; //| PR_POLL_ERR | PR_POLL_HUP | PR_POLL_NVAL;
            pollfds[1].revents = 0;
            nClients = MIN_SERVER_PORTS;

#ifdef USE_SSL
            pollfds[2].fd = sslsrv;
            pollfds[2].events = PR_POLL_READ;// | PR_POLL_EXCEPT;
            pollfds[2].revents = 0;

            pollfds[3].fd = sslsrv6;
            pollfds[3].events = PR_POLL_READ;// | PR_POLL_EXCEPT;
            pollfds[3].revents = 0;
            nClients += SSL_PORT_COUNT;
#endif

#define MIN_PORT_COUNT MIN_SERVER_PORTS + SSL_PORT_COUNT

            offset = 1;
            shift  = 0;

            //clear out invalid fds
            int i = MIN_PORT_COUNT;

            while  ( i < nClients ) {
                if ( pollfds[i].fd == -1 ) {
                    debuglog ( " >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Poll fd is invalid %d , Deleting connection object \n", pollfds[i].fd );
                    pollfds[i].fd = 0;
                }

                i++;
            }


            for ( i = MIN_PORT_COUNT; i < MAXCLIENTS; i++ ) {
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
                        pollfds[i].events  =  PR_POLL_READ | PR_POLL_WRITE; // | PR_POLL_ERR | PR_POLL_HUP | PR_POLL_NVAL;
                        pollfds[i].revents = 0;
                        pollfds[shift].fd = 0;
                        pollfds[shift].events  = 0;
                        pollfds[shift].revents = 0;
                        conn[i] = conn[shift];
                        conn[shift] = 0;
                        nClients++;
                    }
                } else {
                    pollfds[i].events  =  PR_POLL_READ | PR_POLL_WRITE;// | PR_POLL_ERR | PR_POLL_HUP | PR_POLL_NVAL;
                    pollfds[i].revents = 0;
                    nClients++;
                }
            }

            debuglog (  " Shrunk: Number of clients = %d \n", nClients ); //cout << "Number of clients = " << nClients << endl;
            i = 0;
            debuglog (  "DBUG: Active fds=%d After shrinking \n", nClients );

            while  ( i < nClients ) {
                debuglog (  "%d, ", pollfds[i].fd );
                i++;
            }

            shrink = false;
            displayfds = true;
        }

        if ( displayfds ) {
            int l = 0;
            debuglog (  "DBUG: \n" );
            debuglog (  "DBUG: ____________________________________\n" );
            debuglog (  "DBUG: Active fds=%d \n", nClients );

#if 0

            while  ( l < nClients ) {
                debuglog (  "%d, ", pollfds[l].fd );

                if ( pollfds[l].fd == -1 ) {
                    conn_close ( conn[l] );
                    conn[l] = 0;
                    pollfds[l].fd = 0;
                    shrink = true;
                }

                l++;
            }

#endif

            l = 0;
            debuglog (  "DBUG: Active fds=%d After clearing \n", nClients );

            while  ( l < nClients ) {
                debuglog (  "%d, ", pollfds[l].fd );
                l++;
            }

            debuglog (  "\n" );
            debuglog (  "____________________________________\n" );

            if ( shrink )
            { goto shrinkStart; }

            displayfds = false;
        }

        int q = 0;
        while( q < 1 ){
            //debuglog( "DBUG: Polling on : %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d \n", pollfds[q].fd, pollfds[q+1].fd , pollfds[q+2].fd, pollfds[q+3].fd, pollfds[q+4].fd,
            //         pollfds[q+5].fd, pollfds[q+6].fd, pollfds[q+7].fd, pollfds[q+8].fd, pollfds[q+9].fd, pollfds[q+10].fd , pollfds[q+11].fd, pollfds[q+12].fd,
            //         pollfds[q+13].fd, pollfds[q+14].fd, pollfds[q+15].fd );
            q++;
        }

        if ( ( retVal = poll ( pollfds, nClients, 1 ) ) != -1 ) {

            if ( clientmanage ( 2 ) < MAX_BACKLOG ) {
                int old_count = 0;
                int new_count = MIN_PORT_COUNT;

                if ( nClients == MIN_PORT_COUNT || nClients >= MAX_BACKLOG ) {
                    old_count = new_count;
                    new_count = nClients;

                    if ( old_count != new_count )
                    { debuglog ( " Thread connection fds = %d \n", nClients ); }
                }

                /* IPv4 Server Socket */
                if ( pollfds[0].revents & PR_POLL_READ ) {
                    socklen_t addrlen = sizeof ( sockaddr_in );

                    //printf("INFO: Received socket accepting \n");
                    if ( ( client = accept ( srv, ( sockaddr * ) &clientAddr, &addrlen ) ) ) {
                        //TODO:
                        //allowConnect = false;
                        clientmanage ( 1 );
                        allowConnect = true;
                        tempIp = clientAddr.sin_addr.s_addr;
                        char clientAddress[64];
                        inet_ntop ( AF_INET, &clientAddr.sin_addr, clientAddress, 64 );
                        string ipstr = clientAddress;

                        dosMapMutex.lock();
                        MapACL::iterator iter = dosMap.find ( ipstr );

                        if ( iter == dosMap.end() ) {
                            Acl *dosAcl   = new Acl;
                            dosAcl->invert = false;
                            dosAcl->ipv4   = true;
                            dosAcl->subnetmask = false;
                            dosAcl->prefixmask = false;
                            dosAcl->ip = clientAddress;
                            dosAcl->counter = 1;
                            dosMap[ipstr] = dosAcl;
                        } else {
                            if ( iter->second->counter > DOS_THRESHOLD ) {
                                debuglog (  " DOS threshold for the client reached : %s -> %d \n", iter->second->ip.c_str(), DOS_THRESHOLD );
                                //std::cerr << "INFO: DOS treshold reached " << iter->second->ip << ", " << iter->second->counter << std::endl;
                                allowConnect = false;
                            } else {
                                allowConnect = true;
                                iter->second->counter += 1;
                            }
                        }

                        dosMapMutex.unlock();

                        debuglog (  " IPv4 Connection Received from : %s:%d \n", clientAddress, ntohs ( clientAddr.sin_port ) );

                        if ( allowConnect ) {
                            if ( nClients < MAXCLIENTS ) {
                                //if( PR_SetSocketOption(client, &opt) == PR_FAILURE )

                                mode = 1; // 1 to enable non-blocking mode
                                //ioctlsocket(client, FIONBIO, &mode);


                                int flags = fcntl ( client, F_GETFL );
                                flags |= O_NONBLOCK;

                                if ( fcntl ( client, F_SETFL, flags ) != 0 ) {
                                    debuglog (  "ERRR: Unable to set socket option \n" );
                                }

                                debuglog (  "INFO: Received Connection on fd=%d\n", client );
                                pollfds[nClients].fd = client;
                                pollfds[nClients].events = PR_POLL_READ | PR_POLL_WRITE | PR_POLL_ERR ;//| PR_POLL_HUP | PR_POLL_NVAL;
                                pollfds[nClients].revents = 0;

                                if ( ! ( conn[nClients] = new Connection ) ) {
                                    debuglog ( "ERRR: [NEWFAIL] Connection object create failed - %016lld\n", Connection::indexCount );
                                    conn[nClients] = 0;
                                    shutdown ( pollfds[nClients].fd, 2 );
                                    close ( pollfds[nClients].fd );
                                    pollfds[nClients].fd = 0;
                                    pollfds[nClients].events = 0;
                                    pollfds[nClients].revents = 0;
                                    continue;
                                }
				debuglog( "WARN: @@@@@@@@@@@@@@@@@@@ new connection object created = 0x%X , %d \n", conn[nClients], conn[nClients]->index );
                                createCounter ++;
                                conn[nClients]->socketfd = client;
#ifdef USE_SSL
                                conn[nClients]->is_ssl    = 0;
                                conn[nClients]->ssl_accept    = 0;
                                conn[nClients]->ssl    = 0;
#endif
                                conn[nClients]->ip     = tempIp;
                                conn[nClients]->setAuthLvl();
                                nClients++;
				continue;
                            } else {
                                debuglog (  "WARN: Connection closed due to max clients exceeded : %s \n", clientAddress );
                                shutdown ( client, 2 );
                                close ( client );
                            }
                        } else {
                            debuglog (  "WARN: Connection closed because of ACL / DOS list entry: %s \n", clientAddress );
                            shutdown ( client, 2 );
                            close ( client );
                        }
                    }

                    //printf("INFO: Received socket accept finished \n");
                } else {
                    if ( pollfds[0].revents & PR_POLL_NVAL || pollfds[0].revents & PR_POLL_ERR ) {
                        debuglog (  "ERRR: Receive socket invalid !!!!!!!!!!! \n" );
                        close ( pollfds[0].fd );
                        pollfds[0].fd = 0;
                        goto start;
                        //exit(1);
                    }
                }

                /* IPv6 Server Socket */
                if ( pollfds[1].revents & PR_POLL_READ ) {
                    socklen_t addrlen = sizeof ( sockaddr_in6 );

                    if ( ( client6 = accept ( srv6, ( sockaddr * ) &clientAddr6, &addrlen ) ) ) {
                        //TODO:
                        //allowConnect = false;
                        clientmanage ( 1 );
                        allowConnect = true;

                        char clientAddress[64];
                        inet_ntop ( AF_INET6, & ( clientAddr6.sin6_addr ), clientAddress, 64 );
                        string ipstr = clientAddress;

                        dosMapMutex.lock();
                        MapACL::iterator iter = dosMap.find ( ipstr );

                        if ( iter == dosMap.end() ) {
                            Acl *dosAcl   = new Acl;
                            dosAcl->invert = false;
                            dosAcl->ipv4   = true;
                            dosAcl->subnetmask = false;
                            dosAcl->prefixmask = false;
                            dosAcl->ip = clientAddress;
                            dosAcl->counter = 1;
                            dosMap[ipstr] = dosAcl;
                        } else {
                            if ( iter->second->counter > DOS_THRESHOLD ) {
                                debuglog (  "INFO: DOS threshold for the client reached : %s -> %d \n", iter->second->ip.c_str(), DOS_THRESHOLD );
                                //std::cerr << "INFO: DOS treshold reached " << iter->second->ip << ", " << iter->second->counter << std::endl;
                                allowConnect = false;
                            } else {
                                allowConnect = true;
                                iter->second->counter += 1;
                            }
                        }

                        dosMapMutex.unlock();

                        debuglog (  " IPv6 connection received from: %s:%d ...\n", clientAddress, ntohs ( clientAddr6.sin6_port ) );

                        if ( allowConnect ) {
                            if ( nClients < MAXCLIENTS ) {
                                //if( PR_SetSocketOption(client, &opt) == PR_FAILURE )
                                mode = 1;
                                //ioctlsocket( client6, FIONBIO, &mode);
                                int flags = fcntl ( client6, F_GETFL );
                                flags |= O_NONBLOCK;

                                if ( fcntl ( client6, F_SETFL, flags ) != 0 ) {
                                    debuglog (  "ERRR: Unable to set socket option \n" );
                                }

                                debuglog (  "INFO: Received Connection on fd=%d\n", client6 );
                                pollfds[nClients].fd = client6;
                                pollfds[nClients].events = PR_POLL_READ | PR_POLL_WRITE | PR_POLL_ERR ; //| PR_POLL_HUP | PR_POLL_NVAL;
                                pollfds[nClients].revents = 0;

                                //while( !( conn[nClients] = new Connection ) ) {}
                                if ( ! ( conn[nClients] = new Connection ) ) {
                                    debuglog ( "ERRR: [NEWFAIL] Connection object create failed - %016lld\n", Connection::indexCount );
                                    conn[nClients] = 0;
                                    shutdown ( pollfds[nClients].fd, 2 );
                                    close ( pollfds[nClients].fd );
                                    pollfds[nClients].fd = 0;
                                    pollfds[nClients].events = 0;
                                    pollfds[nClients].revents = 0;
                                    continue;
                                }
				debuglog( "WARN: @@@@@@@@@@@@@@@@@@@ new connection object created = 0x%X , %d \n", conn[nClients], conn[nClients]->index );
                                createCounter ++;
                                //conn[nClients] = new Connection;
                                conn[nClients]->socketfd = client6;
#ifdef USE_SSL
                                conn[nClients]->is_ssl    = 0;
                                conn[nClients]->ssl_accept    = 0;
                                conn[nClients]->ssl    = 0;
#endif
                                conn[nClients]->ip     = 0;
                                memcpy ( conn[nClients]->ipv6, &clientAddr6.sin6_addr, sizeof ( clientAddr6.sin6_addr ) );
                                conn[nClients]->setAuthLvl();
                                nClients++;
				continue;
                            } else {
                                debuglog (  "WARN: Connection closed due to max clients exceeded : %s \n", clientAddress );
                                shutdown ( client6, 2 );
                                close ( client6 );
                            }
                        } else {
                            debuglog (  "WARN: Connection closed because of ACL / DOS list entry: %s \n", clientAddress );
                            shutdown ( client6, 2 );
                            close ( client6 );
                        }
                    }

                } else {
                    if ( pollfds[1].revents & PR_POLL_NVAL || pollfds[1].revents & PR_POLL_ERR ) {
                        debuglog (  "ERRR: Receive socket invalid !!!!!!!!!!! \n" );
                        close ( pollfds[1].fd );
                        pollfds[1].fd = 0;
                        goto start;
                        //exit(1);
                    }
                }


#ifdef USE_SSL

                /* SSL IPv4 server socket */
                if ( pollfds[2].revents & PR_POLL_READ ) {
                    socklen_t ssladdrlen = sizeof ( sockaddr_in );

                    debuglog ("INFO: Receive socket accepting on ssl ipv4 fd \n");
                    if ( ( sslclient = accept ( sslsrv, ( sockaddr * ) &sslclientAddr, &ssladdrlen ) ) ) {
                        //TODO:
                        //allowConnect = false;
                        clientmanage ( 1 );
                        allowConnect = true;
                        tempIp = sslclientAddr.sin_addr.s_addr;
                        char clientAddress[64];
                        inet_ntop ( AF_INET, &sslclientAddr.sin_addr, clientAddress, 64 );
                        string ipstr = clientAddress;

                        dosMapMutex.lock();
                        MapACL::iterator iter = dosMap.find ( ipstr );

                        if ( iter == dosMap.end() ) {
                            Acl *dosAcl   = new Acl;
                            dosAcl->invert = false;
                            dosAcl->ipv4   = true;
                            dosAcl->subnetmask = false;
                            dosAcl->prefixmask = false;
                            dosAcl->ip = clientAddress;
                            dosAcl->counter = 1;
                            dosMap[ipstr] = dosAcl;
                        } else {
                            if ( iter->second->counter > DOS_THRESHOLD ) {
                                debuglog (  " DOS threshold for the client reached : %s -> %d \n", iter->second->ip.c_str(), DOS_THRESHOLD );
                                //std::cerr << "INFO: SSL IPv4 DOS treshold reached " << iter->second->ip << ", " << iter->second->counter << std::endl;
                                allowConnect = false;
                            } else {
                                allowConnect = true;
                                iter->second->counter += 1;
                            }
                        }

                        dosMapMutex.unlock();

                        debuglog (  " SSL IPv4 Connection Received from : %s:%d -> %d \n", clientAddress, ntohs ( clientAddr.sin_port ), nClients );

                        if ( allowConnect ) {
                            if ( nClients < MAXCLIENTS ) {
                                //if( PR_SetSocketOption(client, &opt) == PR_FAILURE )

                                mode = 1; // 1 to enable non-blocking mode
                                //ioctlsocket(sslclient, FIONBIO, &mode);
                                int flags = fcntl ( sslclient, F_GETFL );
                                flags |= O_NONBLOCK;

                                if ( fcntl ( sslclient, F_SETFL, flags ) != 0 ) {
                                    debuglog (  "ERRR: Unable to set socket option \n" );
                                }
                                debuglog (  "INFO: Received Connection on ssl ipv4 fd=%d\n", sslclient );
                                pollfds[nClients].fd = sslclient;
                                pollfds[nClients].events = PR_POLL_READ  | PR_POLL_WRITE | PR_POLL_ERR ;
                                pollfds[nClients].revents = 0;

                                //while( !( conn[nClients] = new Connection ) ) {}
                                if ( ! ( conn[nClients] = new Connection ) ) {
                                    debuglog ( "ERRR: [NEWFAIL] Connection object create failed - %016lld\n", Connection::indexCount );
                                    conn[nClients] = 0;
                                    shutdown( pollfds[nClients].fd, 2 );
                                    close ( pollfds[nClients].fd );
                                    pollfds[nClients].fd = 0;
                                    pollfds[nClients].events = 0;
                                    pollfds[nClients].revents = 0;
                                    continue;
                                }
				debuglog( "WARN: @@@@@@@@@@@@@@@@@@@ new connection object created = 0x%X , %d \n", conn[nClients], conn[nClients]->index );
                                createCounter ++;
                                //conn[nClients] = new Connection;
                                conn[nClients]->socketfd = sslclient;
                                conn[nClients]->is_ssl = true;
                                conn[nClients]->ssl_accept = false;
                                conn[nClients]->ip     = tempIp;
                                conn[nClients]->setAuthLvl();

                                conn[nClients]->ssl = SSL_new ( cIp4 );
                                SSL_set_fd ( conn[nClients]->ssl, pollfds[nClients].fd );

                                if ( !conn[nClients]->ssl_accept ) {
                                    if ( SSL_accept ( conn[nClients]->ssl ) <= 0 ) {
                                        debuglog (  "WARN: SSL_accept: failure : %s \n", clientAddress );
                                    } else {
                                        debuglog (  "WARN: SSL_accept: success : %s \n", clientAddress );
                                        conn[nClients]->ssl_accept = true;
                                    }
                                }

                                nClients++;
				continue;
                            } else {
                                debuglog (  "WARN: Connection closed due to max clients exceeded : %s \n", clientAddress );
                                shutdown ( sslclient, 2 );
                                close ( sslclient );
                            }
                        } else {
                            debuglog (  "WARN: Connection closed because of ACL / DOS list entry: %s \n", clientAddress );
                            shutdown ( sslclient, 2 );
                            close ( sslclient );
                        }
                    }

                    //printf("INFO: Received socket accept finished \n");
                } else {
                    if ( pollfds[2].revents & PR_POLL_NVAL || pollfds[2].revents & PR_POLL_ERR ) {
                        debuglog (  "ERRR: Receive socket invalid !!!!!!!!!!! \n" );
                        close ( pollfds[2].fd );
                        pollfds[2].fd = 0;
                        goto start;
                        //exit(1);
                    }
                }

                /* SSL IPv6 server socket */
                if ( pollfds[3].revents & PR_POLL_READ ) {
                    socklen_t addrlen = sizeof ( sockaddr_in6 );
                    debuglog ("INFO: Receive socket accepting on ssl ipv6 fd \n");

                    if ( ( sslclient6 = accept ( sslsrv6, ( sockaddr * ) &sslclientAddr6, &addrlen ) ) ) {
                        //TODO:
                        //allowConnect = false;
                        clientmanage ( 1 );
                        allowConnect = true;

                        char clientAddress[64];
                        inet_ntop ( AF_INET6, & ( sslclientAddr6.sin6_addr ), clientAddress, 64 );
                        string ipstr = clientAddress;

                        dosMapMutex.lock();
                        MapACL::iterator iter = dosMap.find ( ipstr );

                        if ( iter == dosMap.end() ) {
                            Acl *dosAcl   = new Acl;
                            dosAcl->invert = false;
                            dosAcl->ipv4   = true;
                            dosAcl->subnetmask = false;
                            dosAcl->prefixmask = false;
                            dosAcl->ip = clientAddress;
                            dosAcl->counter = 1;
                            dosMap[ipstr] = dosAcl;
                        } else {
                            if ( iter->second->counter > DOS_THRESHOLD ) {
                                debuglog (  "INFO: DOS threshold for the client reached : %s -> %d \n", iter->second->ip.c_str(), DOS_THRESHOLD );
                                //std::cerr << "INFO: SSL IPv6 DOS treshold reached " << iter->second->ip << ", " << iter->second->counter << std::endl;
                                allowConnect = false;
                            } else {
                                allowConnect = true;
                                iter->second->counter += 1;
                            }
                        }

                        dosMapMutex.unlock();

                        debuglog (  " SSL IPv6 connection received from: %s:%d -> %d \n", clientAddress, ntohs ( clientAddr6.sin6_port ), nClients );

                        if ( allowConnect ) {
                            if ( nClients < MAXCLIENTS ) {
                                //if( PR_SetSocketOption(client, &opt) == PR_FAILURE )

                                mode = 1; // 1 to enable non-blocking mode
                                //ioctlsocket(sslclient6, FIONBIO, &mode);
                                int flags = fcntl ( sslclient6, F_GETFL );
                                flags |= O_NONBLOCK;

                                if ( fcntl ( sslclient6, F_SETFL, flags ) != 0 ) {
                                    debuglog (  "ERRR: Unable to set socket option \n" );
                                }
                                debuglog (  "INFO: Received Connection on ssl fd=%d\n", sslclient6 );
                                pollfds[nClients].fd = sslclient6;
                                pollfds[nClients].events = PR_POLL_READ | PR_POLL_WRITE | PR_POLL_ERR ;
                                pollfds[nClients].revents = 0;

                                //while( !( conn[nClients] = new Connection ) ) {}
                                if ( ! ( conn[nClients] = new Connection ) ) {
                                    debuglog ( "ERRR: [NEWFAIL] Connection object create failed - %016lld\n", Connection::indexCount );
                                    conn[nClients] = 0;
                                    shutdown ( pollfds[nClients].fd, 2 );
                                    close ( pollfds[nClients].fd );
                                    pollfds[nClients].fd = 0;
                                    pollfds[nClients].events = 0;
                                    pollfds[nClients].revents = 0;
                                    continue;
                                }
				debuglog( "WARN: @@@@@@@@@@@@@@@@@@@ new connection object created = 0x%X , %d \n", conn[nClients], conn[nClients]->index );
                                createCounter ++;
                                //conn[nClients] = new Connection;
                                conn[nClients]->socketfd   = sslclient6;
                                conn[nClients]->is_ssl     = true;
                                conn[nClients]->ssl_accept = false;
                                conn[nClients]->ip     = 0;
                                memcpy ( conn[nClients]->ipv6, &sslclientAddr6.sin6_addr, sizeof ( sslclientAddr6.sin6_addr ) );
                                conn[nClients]->setAuthLvl();

                                conn[nClients]->ssl = SSL_new ( cIp6 );
                                SSL_set_fd ( conn[nClients]->ssl, conn[nClients]->socketfd );

                                if ( !conn[nClients]->ssl_accept ) {
                                    if ( SSL_accept ( conn[nClients]->ssl ) <= 0 ) {
                                    } else {
                                        conn[nClients]->ssl_accept = true;
                                    }
                                }

                                nClients++;
				continue;
                            } else {
                                debuglog (  "WARN: Connection closed due to max clients exceeded : %s \n", clientAddress );
                                shutdown ( sslclient6, 2 );
                                close ( sslclient6 );
                            }
                        } else {
                            debuglog (  "WARN: Connection closed because of ACL / DOS list entry: %s \n", clientAddress );
                            shutdown ( sslclient6, 2 );
                            close ( sslclient6 );
                        }
                    }

                } else {
                    if ( pollfds[3].revents & PR_POLL_NVAL || pollfds[3].revents & PR_POLL_ERR ) {
                        debuglog (  "ERRR: Receive socket invalid !!!!!!!!!!! \n" );
                        close ( pollfds[3].fd );
                        pollfds[3].fd = 0;
                        goto start;
                        //exit(1);
                    }
                }

#endif
            } /* BACKLOG Check */

            //debuglog ( "DBUG:  Looping from %d to %d \n", MIN_PORT_COUNT, nClients );
            for ( i = MIN_PORT_COUNT ; i < nClients; i++ ) {
                bool isWritable = false;

                if ( pollfds[i].revents & PR_POLL_WRITE )
                {
                    //debuglog (  "DBUG: Fd is now writable \n" );
                    isWritable = true;
                } else {
                    debuglog (  "DBUG: Fd is not writable %d \n", pollfds[i].fd );
                }

                if ( pollfds[i].revents & PR_POLL_NVAL || pollfds[i].revents & PR_POLL_ERR || pollfds[i].revents & PR_POLL_HUP ) {
                    debuglog (  "DBUG: Invalid fd, closing , connection object id = %016lld , %d \n", conn[i] ? conn[i]->index : 0LL, pollfds[i].fd );
                    conn_close ( conn[i] );
                    conn[i] = 0;
                    shutdown ( pollfds[i].fd, 2 );
                    close ( pollfds[i].fd );
                    pollfds[i].fd = 0;
                    pollfds[i].events = 0;
                    pollfds[i].revents = 0;
                    shrink = true;
                    continue;
                }

                if ( !conn[i] ) {
                    debuglog (  "ERRR: Unable to locate connection data %d \n", pollfds[i].fd );
                    shutdown ( pollfds[i].fd, 2 );
                    close ( pollfds[i].fd );
                    pollfds[i].fd = 0;
                    pollfds[i].events = 0;
                    pollfds[i].revents = 0;
                    shrink = true;
                    continue;
                }

                /*if ( ! (pollfds[i].revents & PR_POLL_READ) ){
                    debuglog (  "DBUG: No Read event received %d  \n", pollfds[i].fd);
                }*/

                if ( /*pollfds[i].fd > MIN_PORT_COUNT && */ pollfds[i].revents & PR_POLL_READ ) {

                        debuglog (  "DBUG: Read event received %d ---------------------------------------------------------------------------------\n", pollfds[i].fd);
#ifdef USE_SSL

                    if ( conn[i]->ssl && !conn[i]->ssl_accept ) {
                        int rc = 0;

                        if ( ( rc = SSL_accept ( conn[i]->ssl ) ) == 0 ) {
                            int sslerror = 0;

                            if ( ( sslerror = SSL_get_error ( conn[i]->ssl, rc ) ) == SSL_ERROR_WANT_ACCEPT ) {
                                continue;
                            } else {
                                debuglog (  "ERRR: SSL_accept() gave error %d , sslerror=%d \n", rc, sslerror );
                                conn_close ( conn[i] );
                                conn[i] = 0;
                                shutdown ( pollfds[i].fd, 2 );
                                pollfds[i].fd = 0;
                                pollfds[i].events = 0;
                                pollfds[i].revents = 0;
                                shrink = true;
                                continue;
                            }
                        } else if ( rc < 0 ) {
                            int sslerror = SSL_get_error ( conn[i]->ssl, rc );

                            if ( sslerror == SSL_ERROR_WANT_WRITE || sslerror == SSL_ERROR_WANT_READ ) {
                                debuglog (  "WARN: SSL_accept() gave error but can continue %d, sslerror=%d  \n", rc, sslerror );
                                conn[i]->ssl_accept = true;
                            } else {

                                debuglog (  "ERRR: SSL_accept() gave error %d, sslerror=%d  \n", rc, sslerror );
                                conn_close ( conn[i] );
                                conn[i] = 0;
                                shutdown ( pollfds[i].fd, 2 );
                                pollfds[i].fd = 0;
                                pollfds[i].events = 0;
                                pollfds[i].revents = 0;
                                shrink = true;
                                continue;
                            }
                        } else {
                            conn[i]->ssl_accept = true;
                            //SSL accept done now do SSL_read
                        }
                    }

#endif
                    //HttpReq *temp = &conn[i]->req;
                    HttpReq &REQ  = conn[i]->req;
                    HttpResp &RESP = conn[i]->resp;

                    int tempLen = 0;
#ifdef USE_SSL

                    if ( conn[i]->ssl ) {
                        if ( REQ.hLen <= 0 )
                        { tempLen   = SSL_read ( conn[i]->ssl, & ( REQ.buf[REQ.len] ), MAXBUF - REQ.len ); }
                        else
                        { tempLen   = SSL_read ( conn[i]->ssl, & ( REQ.buf[REQ.hLen] ), MAXBUF - REQ.hLen ); }
                    } else {
#endif
                        debuglog (  "DBUG: Header read, Received Header %d \n", pollfds[i].fd);
                        if ( REQ.hLen <= 0 )
                        { tempLen   = recv ( pollfds[i].fd, & ( REQ.buf[REQ.len] ), MAXBUF - REQ.len, 0 ); }
                        else
                        { tempLen   = recv ( pollfds[i].fd, & ( REQ.buf[REQ.hLen] ), MAXBUF - REQ.hLen, 0 ); }

#ifdef USE_SSL
                    }

#endif
                    debuglog (  "DBUG: Header read, Received Header :\n %d \n", tempLen );
                    if ( tempLen > 0 ) {
                        REQ.len += tempLen; // Total Data Length;
                        debuglog (  "DBUG: Header read, Received Header :\n %s \n", REQ.buf );
                        if ( REQ.hLen <= 0 ) {
                            REQ.readHttpHeader();

                            if ( REQ.isHdrInvalid() ) {
                                REQ.buf[REQ.len] = 0;
                                debuglog (  "ERRR: Header invalid, Received Header :\n %s \n", REQ.buf );
                                conn_close ( conn[i] );
                                conn[i] = 0;
                                shutdown ( pollfds[i].fd, 2 );
                                pollfds[i].fd = 0;
                                pollfds[i].events = 0;
                                pollfds[i].revents = 0;
                                shrink = true;
                                continue;
                            }

                            if ( REQ.hLen <= 0 ) {
                                debuglog (  "WARN: Incomplete header received\n" );
                                //pollfds[i].revents = 0;
                                pollfds[i].revents = pollfds[i].revents & ~PR_POLL_READ;
                                continue;
                            } else {
                                if ( REQ.getMethod() == HTTP_POST ) {
                                    REQ.processHttpPostData ( REQ.hLen, REQ.len - REQ.hLen );

                                    if (  REQ.cLen > REQ.len - REQ.hLen ) {
                                        //pollfds[i].revents = 0;
                                        pollfds[i].revents = pollfds[i].revents & ~PR_POLL_READ;
                                        continue;
                                    }
                                }
                            }
                        } else {
                            if ( REQ.getMethod() == HTTP_POST ) {
                                if ( REQ.cLen > 0 ) {
                                    REQ.processHttpPostData ( REQ.hLen, tempLen );
                                }

                                if ( REQ.cLen > REQ.len - REQ.hLen ) {
                                    //pollfds[i].revents = 0;
                                    pollfds[i].revents = pollfds[i].revents & ~PR_POLL_READ;
                                    continue;
                                }
                            }
                        }

                        MapStrStr *cookies = HttpSession::readCookies ( ( char * ) REQ.getCookie() );
                        conn[i]->sess = 0;

                        if ( cookies ) {
                            MapStrStr::iterator itr = cookies->find ( "SID" );

                            if ( itr != cookies->end() ) {
                                conn[i]->sess = hSMgr->getSession ( conn[i]->ip, itr->second );

                                if ( conn[i]->sess ) {
                                    conn[i]->setAuthLvl();
                                    debuglog (  "INFO: Session Active, SID: %s -> Auth Level : %s \n",
                                                conn[i]->sess->sid.c_str(), conn[i]->sess->getVariable ( "auth" ) );
                                }
                            } else {
                                debuglog (  "ERRR: Unable to find SID in cookies \n" );
                            }

                            delete cookies;
                        } else {
                            debuglog (  "DBUG: Cookies not present \n" );
                        }

                        if ( !conn[i]->sess ) {
                            debuglog (  "WARN: No valid Session Present, creating new \n" );
                            time_t now = time ( NULL );
                            now += 86400;
                            conn[i]->sess = hSMgr->startSession ( conn[i]->ip, now );
                            //TODO: We don't need to save the entire map everytime a new connection comes in
                            //hSMgr->saveSession();
                        }

                        if ( conn[i]->sess ) {
                            //TODO:
                            //PRStatus fInfoStatus = PR_SUCCESS;
                            bool isForbidden = false;
                            bool notFound = false;

                            char *fileType = REQ.getReqFileType();
                            int ftn = strlen ( fileType );

                            if ( fileType[ftn - 1] == '\n' ) {
                                fileType[ftn - 1] = 0;
                                fileType[ftn - 2] = 0;
                            }

                            debuglog (  "DBUG: Requesting %s type \n", fileType );

                            if ( ( strcasecmp ( fileType, ".html" ) == 0 ) ||
                                    ( strcasecmp ( fileType, ".js" )   == 0 ) ||
                                    ( strcasecmp ( fileType, ".htm" )  == 0 ) ||
                                    ( strcasecmp ( fileType, ".xyz" )  == 0 ) ||
                                    ( strcasecmp ( fileType, ".css" )  == 0 )
                               ) {
                                char *authFile = REQ.getReqFileAuth ( conn[i]->authLvl );
                                conn[i]->file  = fopen ( authFile, "rb+" );
                                if( conn[i]->file )
                                    conn[i]->isFileOpened = true;
                                //fInfoStatus    = PR_GetFileInfo64 ( * ( conn[i]->file ), & ( conn[i]->fInfo ) );
                                debuglog (  "WARN: Requesting static/scriptfile file type : %s \n", authFile );

                                if ( conn[i]->file && strcasecmp ( fileType, ".xyz" ) != 0 ) {
                                    notFound = true;
                                }
                            } else if ( ( strcasecmp ( fileType, ".png" )   == 0 ) ||
                                        ( strcasecmp ( fileType, ".jpg" )   == 0 ) ||
                                        ( strcasecmp ( fileType, ".ico" )   == 0 ) ||
                                        ( strcasecmp ( fileType, ".bmp" )   == 0 ) ||
                                        ( strcasecmp ( fileType, ".sthtm" )   == 0 ) ||
                                        ( strcasecmp ( fileType, ".sthtml" )   == 0 ) ||
                                        ( strcasecmp ( fileType, ".jpeg" )  == 0 ) ||
                                        ( strcasecmp ( fileType, ".bin" )  == 0 ) ) {
                                char *authFile = REQ.getReqFileAuth ( );
                                conn[i]->file = fopen ( authFile, "rb+" );
                                if( conn[i]->file )
                                    conn[i]->isFileOpened = true;
                                debuglog (  "WARN: Requesting static file : %s \n", authFile );
                                //fInfoStatus    = PR_GetFileInfo64 ( * ( conn[i]->file ), & ( conn[i]->fInfo ) );

                                if ( !conn[i]->file ) {
                                    notFound = true;
                                }
                            } else {
                                isForbidden = true;
                                debuglog (  "WARN: Requesting unsupported file type \n" );
                                //HttpResp *tempResp   = &conn[i]->resp;
                                //tempResp->setContentLen( conn[i]->fid->fileSize);
                                RESP.setStatus ( 403 );
                                RESP.setContentLen ( 0 );
                                RESP.setAddOn ( 1 );
                                pollfds[i].events  = PR_POLL_READ | PR_POLL_WRITE;
                                RESP.setContentType ( ( char * ) identifyContentType ( REQ.getReqFile() ) );
                                conn[i]->len = RESP.getHeader ( ( char * ) conn[i]->buf );
                                conn[i]->len += conn[i]->sess->dumpSessionCookies ( ( char * ) & ( conn[i]->buf[conn[i]->len] ) );
                                conn[i]->len += RESP.finishHdr ( ( char * ) & ( conn[i]->buf[conn[i]->len] ) );
                                //27/04/2025
                                conn[i]->hLen = conn[i]->len;
                                debuglog (  " Request file : %s -> Status code = %d \n", ( char * ) conn[i]->req.getReqFile(), 403 );
                                debuglog (  "XTRA: Response Header: \n%s\n", ( char * ) conn[i]->buf );
                                debuglog (  "XTRA: ____________________________________\n" );
                            }

                            /*
                             * Validate Authorization
                             */
                            if ( conn[i]->file  ) {
                                //TODO:
                                //HttpResp *tempResp   = &conn[i]->resp;
                                //tempResp->setContentLen( conn[i]->fid->fileSize);

                                if (fseek(conn[i]->file, 0, SEEK_END) != 0) { debuglog("ERRR: fseek failed"); fclose(conn[i]->file); conn[i]->file = 0; }
                                // Get the current file pointer position
                                long fileSize = ftell(conn[i]->file);
                                if (fileSize == -1) { debuglog("ERRR: ftell failed"); fclose(conn[i]->file); conn[i]->file = 0; fileSize = 0; }
                                if (fseek(conn[i]->file, 0, SEEK_SET) != 0) { debuglog("ERRR: fseek failed"); fclose(conn[i]->file); conn[i]->file = 0; }

                                RESP.setContentLen ( fileSize );
                                RESP.setAddOn ( 1 );
                                pollfds[i].events  = PR_POLL_READ | PR_POLL_WRITE | PR_POLL_ERR ;// | PR_POLL_HUP | PR_POLL_NVAL ;
                                RESP.setContentType ( ( char * ) identifyContentType ( REQ.getReqFile() ) );
                                conn[i]->len = RESP.getHeader ( ( char * ) conn[i]->buf );
                                conn[i]->len += conn[i]->sess->dumpSessionCookies ( ( char * ) & ( conn[i]->buf[conn[i]->len] ) );
                                conn[i]->len += RESP.finishHdr ( ( char * ) & ( conn[i]->buf[conn[i]->len] ) );
                                //27/04/2025
                                conn[i]->hLen = conn[i]->len;
                                debuglog (  " Request file : %s -> Status code = %d \n", ( char * ) REQ.getReqFile(), RESP.getStatus() );
                                debuglog (  "XTRA: Response Header: \n%s\n", ( char * ) conn[i]->buf );
                                debuglog (  "XTRA: ____________________________________\n" );
                            } else {
                                if ( notFound || isForbidden ) {
                                    debuglog (  "ERRR: Static doc : '%s' NOT FOUND\n",
                                                REQ.getReqFileAuth() );

                                    if ( notFound )
                                    { sendConnRespHdr ( conn[i], 404 ); }
                                    else
                                    { sendConnRespHdr ( conn[i], 403 ); }

                                    conn_close ( conn[i] );
                                    conn[i] = 0;
                                    shutdown ( pollfds[i].fd, 2 );
                                    close ( pollfds[i].fd );
                                    pollfds[i].fd = 0;
                                    pollfds[i].events = 0;
                                    pollfds[i].revents = 0;
                                    shrink = true;
                                    continue;
                                }


                                if ( MAX_THREADS == 0 ) {
                                    debuglog (  "ERRR: Dynamic Page (in Static server mode): '%s' \n",
                                                REQ.getReqFileAuth() );

                                    sendConnRespHdr ( conn[i], 404 );
                                    conn_close ( conn[i] );
                                    conn[i] = 0;
                                    shutdown ( pollfds[i].fd, 2 );
                                    close ( pollfds[i].fd );
                                    pollfds[i].fd = 0;
                                    pollfds[i].events = 0;
                                    pollfds[i].revents = 0;
                                    shrink = true;
                                    continue;
                                } else {
                                    //Only forward .xyz plugin associated files to backend threads
                                    HttpHandler *hHdlr = HttpHandler::createInstance();
                                    PluginHandler *pluginHandler = ( PluginHandler * ) hHdlr->getHandler ( REQ.getReqFile() );
                                    debuglog (  "ERRR: Dynamic Page : '%s' \n",
                                                REQ.getReqFile() );

                                    if ( ! isForbidden && strcasecmp ( fileType, ".xyz" ) == 0 && pluginHandler ) {

                                        mode = 1;
                                        //ioctlsocket( conn[i]->socketfd, FIONBIO, &mode);
                                        int flags = fcntl ( conn[i]->socketfd, F_GETFL );
                                        flags &= ~O_NONBLOCK;

                                        if ( fcntl ( conn[i]->socketfd, F_SETFL, flags ) != 0 ) {
                                            debuglog (  "ERRR: Unable to set socket option \n" );
                                        }

                                        debuglog("WARN: assigning task to thread as this is dynamic page \n");
                                        conn[i]->cmd  = THREAD_CMD_PTASK;
                                        tMgr->assignTask ( conn[i] );
                                        assignCounter ++;
                                        conn[i]       = 0;
                                        pollfds[i].fd = 0;
                                        pollfds[i].events = 0;
                                        pollfds[i].revents = 0;
                                        shrink = true;
                                        continue;
                                    } else {
                                    	debuglog (  "ERRR: Dynamic Page : '%s' NOT FOUND \n",
                                                REQ.getReqFile() );

                                        sendConnRespHdr ( conn[i], 404 );
                                        conn_close ( conn[i] );
                                        conn[i] = 0;
                                        shutdown ( pollfds[i].fd, 2 );
                                        close ( pollfds[i].fd );
                                        pollfds[i].fd = 0;
                                        pollfds[i].events = 0;
                                        pollfds[i].revents = 0;
                                        shrink = true;
                                        continue;
                                    }
                                }
                            }
                        }

                        fflush ( stderr );
                    } 
		    else if( tempLen == 0 ){
		    
		    } else {

#ifdef USE_SSL

                        if ( conn[i]->ssl ) {
                            if ( SSL_get_error ( conn[i]->ssl, tempLen ) == SSL_ERROR_WANT_READ )
                            { /*goto writesection;*/ }
                            else {
                                debuglog (  "ERRR: SSL_read Error:  0x%X, '%s' file %d -> socket %d pollfd %d \n", conn[i], 
                                            conn[i]->req.getReqFileAuth(), conn[i]->file, conn[i]->socketfd, pollfds[i].fd );
                                conn_close ( conn[i] );
                                conn[i] = 0;
                                shutdown ( pollfds[i].fd, 2 );
				close(pollfds[i].fd);
                                pollfds[i].fd = 0;
                                pollfds[i].events = 0;
                                pollfds[i].revents = 0;
                                shrink = true;
                                continue;
                            }
                        } else {
#endif
                            debuglog("ERRR: Non-SSL read error: 0x%X %d %d\n", conn[i], i, __LINE__);
                            conn_close ( conn[i] );
                            conn[i] = 0;
                            shutdown ( pollfds[i].fd, 2 );
			    close(pollfds[i].fd);
                            pollfds[i].fd = 0;
                            pollfds[i].events = 0;
                            pollfds[i].revents = 0;
                            shrink = true;
                            continue;
#ifdef USE_SSL
                        }

#endif
                    }

                    if ( pollfds[i].fd > 0 && isWritable ) {
                        pollfds[i].revents = pollfds[i].revents & ~PR_POLL_READ;
                    }
                }

                if( !(pollfds[i].revents & PR_POLL_WRITE) ){
                    debuglog ( "DBUG: No write event received %d\n",pollfds[i].fd );
                }

                if ( /*pollfds[i].fd > MIN_PORT_COUNT && */ pollfds[i].revents & PR_POLL_WRITE && conn[i] && conn[i]->file /*&& * ( conn[i]->file ) > MIN_PORT_COUNT */) {
#ifdef USE_SSL

                    if ( conn[i]->ssl && !conn[i]->ssl_accept ) {
                        int rc = 0;

                        if ( ( rc = SSL_accept ( conn[i]->ssl ) ) == 0 ) {
                            int sslerror = 0;

                            if ( ( sslerror = SSL_get_error ( conn[i]->ssl, rc ) ) == SSL_ERROR_WANT_ACCEPT ) {
                                debuglog (  "WARN: SSL_accept() gave error but can continue to accept %d, sslerror=%d  \n", rc, sslerror );
                                continue;
                            } else {
                                debuglog (  "ERRR: SSL_accept() gave error %d , sslerror=%d \n", rc, sslerror );
                                conn_close ( conn[i] );
                                conn[i] = 0;
                                shutdown ( pollfds[i].fd, 2 );
				close( pollfds[i].fd);
                                pollfds[i].fd = 0;
                                pollfds[i].events = 0;
                                pollfds[i].revents = 0;
                                shrink = true;
                                continue;
                            }
                        } else if ( rc < 0 ) {
                            int sslerror = SSL_get_error ( conn[i]->ssl, rc );

                            if ( sslerror == SSL_ERROR_WANT_WRITE || sslerror == SSL_ERROR_WANT_READ ) {
                                debuglog (  "WARN: SSL_accept() gave error but can continue %d, sslerror=%d  \n", rc, sslerror );
                                conn[i]->ssl_accept = true;
                            } else {

                                debuglog (  "ERRR: SSL_accept() gave error %d, sslerror=%d  \n", rc, sslerror );
                                conn_close ( conn[i] );
                                conn[i] = 0;
                                shutdown ( pollfds[i].fd, 2 );
				close( pollfds[i].fd);
                                pollfds[i].fd = 0;
                                pollfds[i].events = 0;
                                pollfds[i].revents = 0;
                                shrink = true;
                                continue;
                            }
                        } else {
                            conn[i]->ssl_accept = true;
                            //SSL accept done now do SSL_read
                        }
                    }

#endif
                    /*
                    if( conn[i]->sent <= 0 ){
                        //27/04/2025
                        //conn[i]->hLen = conn[i]->len;
                        //conn[i]->len  += conn[i]->resp.getContentLen();
                        fseek( conn[i]->file, 0, SEEK_END );
                        int filesize = ftell(conn[i]->file);
                        fseek( conn[i]->file, 0, SEEK_SET);
                        debuglog(" DBUG: Initial file transfer status: fd:%d, hdr:%d, sent:%d, len:%d, fz:%d \n", pollfds[i].fd, conn[i]->hLen, conn[i]->sent, conn[i]->len, filesize );
                    }
                    */

                    debuglog("DBUG: Send info : fd:%d, hdr:%d, sent:%d, len:%d\n", pollfds[i].fd, conn[i]->hLen, conn[i]->sent, conn[i]->len );
                    int len = conn[i]->len;
                    debuglog("DBUG: Reading bytes fd:%d, len:%d, file:0x%x \n",pollfds[i].fd, len, conn[i]->file  );
                    if ( len < SMALLBUF && conn[i]->file && conn[i]->sent < conn[i]->resp.getContentLen() + conn[i]->hLen ) {
                        //read in some more data
                        int temp = ftell(conn[i]->file);
                        debuglog("DBUG: >>>>>>>> Reading bytes from %d position in file\n", temp );
                        fread ( & ( conn[i]->buf[len] ), SMALLBUF - len, 1, conn[i]->file );
                        temp = ftell(conn[i]->file) - temp;

                        //writeToFile( tmpFile2, &(conn[i]->buf[len]), temp );

                        //debuglog(" DBUG: Read bytes = %d \n", temp );

                        debuglog("DBUG: Read bytes info : fd:%d, len:%d, temp:%d, SB-len:%d, fpos:%d file:0x%x\n",pollfds[i].fd, len, temp, SMALLBUF-len, ftell(conn[i]->file), conn[i]->file  );
                        if( temp > 0 ) {
                            len += temp;
                            conn[i]->len = len;
                            debuglog("DBUG: Reading bytes info : fd:%d, len:%d, connLen:%d, contentLen:%d \n",pollfds[i].fd, len, conn[i]->len, conn[i]->resp.getContentLen()  );
                        } else {

                            if( feof(conn[i]->file) ){
                                debuglog("DBUG: EOF reached \n");
                                fclose ( conn[i]->file );
                                conn[i]->file = 0;
                            } else if( ferror(conn[i]->file ) ){
                                debuglog("DBUG: ERROR using the file \n");
                                fclose ( conn[i]->file );
                                conn[i]->file = 0;
                            } else {
                                debuglog("DBUG: Not eof or error \n");
                            }
                        }
                    }

                    if ( conn[i]->sent < conn[i]->hLen + conn[i]->resp.getContentLen() ) {
                        int bytesW = 0;

                        //do {
                            int temp = 0;
#ifdef USE_SSL

                            if ( conn[i]->ssl ) {
                                temp = SSL_write ( conn[i]->ssl, & ( conn[i]->buf[bytesW] ), conn[i]->len - bytesW );
                                debuglog("DBUG: Sent temp:%d , bytesW:%d , len:%d, connLen:%d bytes (SSL)\n", temp, bytesW, len, conn[i]->len );
                                if ( temp  >  0 ) {
                                    bytesW += temp;
                                    //conn[i]->bufNotSent = conn[i]->len - bytesW;
                                    //memcpy( conn[i]->buf, &conn[i]->buf[conn[i]->len-conn[i]->bufNotSent], conn[i]->bufNotSent );
                                    debuglog("DBUG: Wrote bytes (SSL) = %d\n", temp );
                                } else {
                                    int rc = 0;

                                    if ( ( rc = SSL_get_error ( conn[i]->ssl, temp ) ) == SSL_ERROR_WANT_WRITE ) {
                                        memcpy ( conn[i]->buf, & ( conn[i]->buf[bytesW] ), len - bytesW );
                                        debuglog("DBUG: SSL error (SSL) %d, wrote=%d len=%d, bytesW=%d\n", rc, temp, len, bytesW );
                                        //break;
                                        (conn[i]->ssl_failures)++;
                                    } else {
                                        //fclose(conn[i]->file);
                                        //conn[i]->file = 0;
                                        (conn[i]->ssl_failures)++;
                                        debuglog("ERRR: -999 (SSL) scenario occurred (SSL)\n");
                                        bytesW = -999;
                                        //break; delete
                                    }
                                }
                            } else {
#endif
                                //temp = PR_Write ( conn[i]->socket, conn[i]->buf, conn[i]->len );
                                temp = send ( conn[i]->socketfd, &(conn[i]->buf[bytesW]), conn[i]->len-bytesW, 0 );
                                debuglog("DBUG: Sent temp:%d , bytesW:%d , len:%d, connLen:%d bytes (SSL)\n", temp, bytesW, len, conn[i]->len );
                                if ( temp >= 0 )
                                {
                                    //writeToFile( tmpFile, &(conn[i]->buf[bytesW]), temp );
                                    bytesW += temp;
                                    //conn[i]->bufNotSent = conn[i]->len - bytesW;
                                    //memcpy( conn[i]->buf, &conn[i]->buf[conn[i]->len-conn[i]->bufNotSent], conn[i]->bufNotSent );
                                    debuglog("DBUG: Wrote bytes = %d \n", temp );
                                }
                                else {
                                    //PRErrorCode error = PR_GetError();

                                    if ( temp == -1 ) {
                                            //CHECK
                                        //fclose(conn[i]->file);
                                        //conn[i]->file = 0;
                                        //debuglog("ERRR: int_ERROR on send scenario occurred \n");
                                        int errorCode = errno;
                                        if (errorCode == ECONNRESET ) //|| errorCode == WSAENETRESET || errorCode == WSAECONNABORTED)
                                        {
                                            debuglog("ERRR: The connection has been closed by the peer.\n");
                                        }
                                        else
                                        {
                                            debuglog("ERRR: Send failed with error code: %d\n", errorCode);
                                        }
                                        //memcpy ( conn[i]->buf, & ( conn[i]->buf[bytesW] ), len - bytesW );
                                        //break;
                                    } else {
                                        //fclose(conn[i]->file);
                                        //conn[i]->file = 0;
                                        debuglog("ERRR: -999 scenario occurred \n");
                                        bytesW = -999;
                                        //break;
                                    }
                                    (conn[i]->failures)++;
                                }

#ifdef USE_SSL
                            }

#endif
                       // } while ( bytesW < len );

                        if ( bytesW != -999 ) {
                            debuglog("DBUG: Sent? temp:%d , bytesW:%d , len:%d, connLen:%d bytes\n", temp, bytesW, len, conn[i]->len );
                            conn[i]->len   = len - bytesW;
                            conn[i]->sent += bytesW;
                            debuglog("DBUG: Sent? temp:%d , bytesW:%d , len:%d, connLen:%d bytes\n", temp, bytesW, len, conn[i]->len );
                            if( conn[i]->len == 0 ){
                                debuglog("INFO: Sent the complete data in the buffer \n");
                            } else if (conn[i]->len > 0 ){
                                debuglog("INFO: Couldn't Sent the complete data in the buffer \n");
                            } else {
                                debuglog("INFO: Sent the more data than there was in the buffer \n");
                            }
                        } else {
#ifdef USE_SSL

                            if ( conn[i]->ssl ){
                                if( conn[i]->ssl_failures > 100 ) {
                                    debuglog (  "ERRR: SSL_write Error: '%s' file %d -> socket %d pollfd %d \n",
                                                conn[i]->req.getReqFileAuth(), conn[i]->file, conn[i]->socketfd, pollfds[i].fd );
                                    fclose(conn[i]->file);
                                    conn[i]->file = 0;
                                    conn_close ( conn[i] );
                                    conn[i] = 0;
                                    shutdown ( pollfds[i].fd, 2 );
				    close( pollfds[i].fd);
                                    pollfds[i].fd = 0;
                                    pollfds[i].events = 0;
                                    pollfds[i].revents = 0;
                                    shrink = true;
                                    continue;
                                }
                            } else {
#endif
                                if( conn[i]->failures > 100 ){
                                    debuglog (  "ERRR: Socket Write Error: '%s' file %d -> socket %d pollfd %d \n",
                                                conn[i]->req.getReqFileAuth(), conn[i]->file, conn[i]->socketfd, pollfds[i].fd );
                                    fclose(conn[i]->file);
                                    conn[i]->file = 0;
                                    conn_close ( conn[i] );
                                    conn[i] = 0;
                                    shutdown ( pollfds[i].fd, 2 );
				    close( pollfds[i].fd);
                                    pollfds[i].fd = 0;
                                    pollfds[i].events = 0;
                                    pollfds[i].revents = 0;
                                    shrink = true;
                                    continue;
                                }
#ifdef USE_SSL
                            }

#endif
                        }
                    } else {
                        //debuglog (  "\nINFO: --------------------------------------------------------------------------\n" );
                        //debuglog (  "xxxxINFO: Sent File='%s' Fd=%d Total bytes=%d (including header)\n", conn[i]->req.getReqFile(), pollfds[i].fd,  conn[i]->sent );
                        debuglog (  " [Complete] Sent File='%s' Fd=%d Total bytes=%d (including header)\n", conn[i]->req.getReqFile(), pollfds[i].fd,  conn[i]->sent );
                        //debuglog (  "INFO: --------------------------------------------------------------------------\n\n\n" );
                        if( conn[i] ){
                            fclose( conn[i]->file );
			    conn[i]->file = 0;
                            conn_close ( conn[i] );
                            conn[i] = 0;
                        }
                        shutdown ( pollfds[i].fd, 2 );
                        close( pollfds[i].fd );
                        pollfds[i].fd = 0;
                        pollfds[i].events = 0;
                        pollfds[i].revents = 0;
                        shrink = true;
                        continue;
                    }
                } else {
                    if ( conn[i] && conn[i]->file == 0 && conn[i]->isFileOpened == true ) {
//#if 0
                        debuglog("ERRR: Closing connection, 0x%x, %d \n",conn[i], pollfds[i].fd );
                        conn_close ( conn[i] );
                        conn[i] = 0;
                        shutdown ( pollfds[i].fd, 2 );
                        close( pollfds[i].fd );
                        pollfds[i].fd = 0;
                        pollfds[i].events = 0;
                        pollfds[i].revents = 0;
                        shrink = true;
                        continue;
//#endif
                    }
                }
                if( pollfds[i].fd != 0 )
                    pollfds[i].events  =  PR_POLL_READ | PR_POLL_WRITE; //| PR_POLL_HUP | PR_POLL_ERR | PR_POLL_NVAL;
                else
                    pollfds[i].events  = 0;
                pollfds[i].revents = 0;
                //debuglog("DBUG: setting all  events on fd %d ", pollfds[i].fd);
            }

        } else {
            //debuglog("DBUG: last error on WSAPoll() is %d \n", WSAGetLastError());
            if ( retVal == -1 ) {
                //PR_Sleep ( 1 );
                std::this_thread::sleep_for ( std::chrono::microseconds ( 100 ) );
                int r = MIN_PORT_COUNT;
                bool mustShrink = false;
                while( r < 1024){
                    if( pollfds[r].fd !=0 && pollfds[r].revents & (PR_POLL_ERR | PR_POLL_HUP | PR_POLL_NVAL ) ){
                        debuglog("DBUG: #################### removing fd from polling %d ", pollfds[r].fd);
                        close(pollfds[r].fd);
                        //conn_close( conn[r] );
                        //conn[r] = 0;
                        pollfds[r].fd = 0;
                        pollfds[r].events = 0;
                        pollfds[r].revents = 0;
                        mustShrink = true;

                    }
                    r++;
                }

                if( mustShrink ){
                    shrink = true;
                }
                debuglog (  "ERRR: Poll failed \n" );
                if( ( pollfds[0].revents & (PR_POLL_ERR | PR_POLL_HUP | PR_POLL_NVAL ) ) || (pollfds[1].revents & (PR_POLL_ERR | PR_POLL_HUP | PR_POLL_NVAL ) ) ){
                    debuglog (  "ERRR: Poll failed on listening sockets \n" );
                    return 10;
                }
            }
        }
            q++;
            if( q % 1000 == 0 )
                std::cerr<<" *********WARN : Delete connection stats "<<assignCounter<<"/"<<deleteCounter<<"/"<<createCounter<<std::endl;
    }

    shutdown ( srv, 2 );
    close ( srv );
    shutdown ( srv6, 2 );
    close ( srv6 );
#ifdef USE_SSL
    shutdown ( sslsrv, 2 );
    close ( sslsrv );
    shutdown ( sslsrv6, 2 );
    close ( sslsrv6 );
    SSL_CTX_free ( cIp4 );
    SSL_CTX_free ( cIp6 );
#endif
    PR_Cleanup();
}

