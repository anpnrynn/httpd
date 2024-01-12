#include <threadmgr.h>
#include <httphandlers.h>
#include <prwrapper.h>
#include <tools.h>
#include <pass.h>
#include <defines.h>
#include <session.h>

#include <chrono>

//this gets reset by main()
int MAX_THREADS = 3;

CmdPipe::CmdPipe ( int id ) {
    cmdQueue = new CmdQueue();
    cmdQueueId = id;
    usage    = 0;
}

CmdPipe::~CmdPipe() {
    delete cmdQueue;
}

void CmdPipe::pushCmd ( Connection *conn ) {
    if ( !conn )
    { return; }

    lock.lock();
    cmdQueue->push ( conn );
    fprintf ( stderr, "INFO: Pushing Connection Data to CmdQueue = %d, Size=%ld\n", cmdQueueId, cmdQueue->size() );
    usage++;
    lock.unlock();
}

Connection* CmdPipe::popCmd () {
    Connection* conn = NULL;
    lock.lock();

    if ( cmdQueue->size() > 0 && ( ( conn = cmdQueue->front() ) != NULL ) ) {
        cmdQueue->pop();
        fprintf ( stderr, "INFO: Poping Connection from CmdQueue=%d, Size= %ld\n", cmdQueueId, cmdQueue->size() );
    }

    lock.unlock();
    return conn;
}

void CmdPipe::reduceUsage() {
    lock.lock();
    usage--;
    lock.unlock();
}

int CmdPipe::getUsage () {
    lock.lock();
    int temp = usage;
    lock.unlock();
    return temp;
}

ThreadMgr *ThreadMgr::tMgr = 0;

ThreadMgr::ThreadMgr() {
    int i ;
    cIndex = 0;

    for ( i = 0; i < MAX_THREADS; i++ ) {
        cmdPipe[i] = new CmdPipe ( i );
        index[i]   = i;
        db[i] = NULL;
        //int rc = sqlite3_open(INFO_STORE, &db[i], getEncryptionStructure(ENC_KEY_DATABASE) );
        int rc = sqlite3_open ( INFO_STORE, &db[i] );

        if ( rc ) {
            fprintf ( stderr, "ERRR: Unable to open db, Corrupted ? \n" );
            sqlite3_close ( db[i] );
            db[i] = NULL;
        }
    }
}

ThreadMgr::~ThreadMgr() {
}

ThreadMgr* ThreadMgr::createInstance() {
    if ( ! tMgr ) {
        tMgr = new ThreadMgr();
    }

    return tMgr;
}

void ThreadMgr::startThreads() {
    int i;

    for ( i = 0; i < MAX_THREADS; i++ ) {
#ifdef  USE_CPP11THREAD
        tdata[i]   = new std::thread ( ThreadMgr::httpthread, &index[i] );
        fprintf ( stderr, "INFO: Created CPP 11 Thread with Index=%d \n", i );
#else
#ifndef USE_PTHREAD
        tdata[i]   = PR_CreateThread ( PR_SYSTEM_THREAD, ThreadMgr::httpthread, &index[i],
                                       PR_PRIORITY_NORMAL, PR_GLOBAL_THREAD, PR_JOINABLE_THREAD, 8388608 );

        if ( !tdata[i] ) {
            fprintf ( stderr, "ERRR: Unable to create thread \n" );
        } else {
            fprintf ( stderr, "INFO: Created Thread with Index=%d \n", i );
        }

#else
        int rc = pthread_create ( &tdata[i], NULL, ThreadMgr::httpthread, &index[i] );

        if ( rc != 0 ) {
            fprintf ( stderr, "ERRR: Unable to create thread \n" );
        } else {
            fprintf ( stderr, "INFO: Created Thread with Index=%d \n", i );
        }

#endif
#endif
    }
}

void ThreadMgr::stopThreads() {
    fprintf ( stderr, "INFO: Stopping all threads \n" );
    int i;

    for ( i = 0; i < MAX_THREADS; i++ ) {
        Connection *conn = new Connection();
        conn->cmd = THREAD_CMD_EXIT;
        cmdPipe[i]->pushCmd ( conn );
#ifdef  USE_CPP11THREAD

        if ( tdata[i] ) {
            tdata[i]->join();
            fprintf ( stderr, "DBUG: Stopped thread %d \n", i );
            tdata[i] = 0;
        }

#else
#ifndef USE_PTHREAD
        PR_JoinThread ( tdata[i] );
        fprintf ( stderr, "DBUG: Stopped thread %d \n", i );
        tdata[i] = NULL;
#else
        pthread_join ( tdata[i], NULL );
        fprintf ( stderr, "DBUG: Stopped thread %d \n", i );
        tdata[i] = 0;
#endif
#endif
    }
}

#ifdef  USE_CPP11THREAD
void ThreadMgr::httpthread ( int *data )
#else
#ifndef USE_PTHREAD
void ThreadMgr::httpthread ( void *data )
#else
void* ThreadMgr::httpthread ( void *data )
#endif
#endif
{
    int* index = ( int* ) data;
    //ThreadMgr   *mgr  = ThreadMgr::createInstance();
    ThreadMgr   *mgr  = ThreadMgr::tMgr;
    Connection  *conn = NULL;
    HttpHandler *hHdlr = HttpHandler::createInstance();
    int cmd   = THREAD_CMD_SLEEP;
    int aLvl  = 1;
    //printf("In thread %d \n",*index);
    sqlite3 *db  = NULL;

    do {
        conn = NULL;

        if ( ( conn = mgr->cmdPipe[*index]->popCmd() ) == NULL || conn == ( Connection * ) 0xFFFFFFFF ) {
            std::this_thread::sleep_for ( std::chrono::milliseconds ( 10 ) );
            continue;
        } else {
            //Process the request here
            fprintf ( stderr, "DBUG: Thread %d received task %llu\n", *index, ( unsigned long long int ) conn );
            cmd = conn->cmd;

            if ( cmd == THREAD_CMD_PTASK ) {
                aLvl = atoi ( conn->sess->getVariable ( "auth" ) );

                if ( aLvl == 0 )
                { aLvl = 1; }

                PluginHandler *temp = ( PluginHandler * ) hHdlr->getHandler ( conn->req.getReqFile() );
                HttpResp *tempResp   = &conn->resp;
                tempResp->setAddOn ( 1 );

                if ( temp && ( aLvl >= ( int ) temp->authLvlReq ) ) {
                    db   = mgr->db[*index];

                    if ( !db ) {
                        tempResp->setContentLen ( 0 );
                        sendConnRespHdr ( conn, HTTP_RESP_INTERNALERR );
                        PR_Shutdown ( conn->socket, PR_SHUTDOWN_BOTH );
                        fprintf ( stderr, "INFO: Shuting down socket \n" );
                        PR_Close ( conn->socket );
                        conn->socket = NULL;
                        delete  conn;
                        continue;
                    } else {
                        tempResp->setContentLen ( -1 );
                        tempResp->setStatus ( HTTP_RESP_OK );
                        conn->db    = db;
                        conn->udata = NULL;
                        int rc = temp->processReq ( conn );
                        fprintf ( stderr, "INFO: Plugin execution completed = %d \n", rc );
                        PR_Shutdown ( conn->socket, PR_SHUTDOWN_BOTH );
                        fprintf ( stderr, "INFO: Shuting down socket \n" );
                        PR_Close ( conn->socket );
                        fprintf ( stderr, "INFO: Closing socket \n" );
                        conn->socket = NULL;
                        delete ( conn );
                        fprintf ( stderr, "INFO: Deleted connection \n" );
                    }
                } else {
                    fprintf ( stderr, "ERRR: Unable to find handler: %s \n", conn->req.getReqFile() );
                    tempResp->setContentLen ( 0 );
                    sendConnRespHdr ( conn, HTTP_RESP_NOTFOUND );
                    PR_Close ( conn->socket );
                    conn->socket = NULL;
                    delete ( conn );
                }
            }

            mgr->cmdPipe[*index]->reduceUsage();
        }
    } while ( cmd != THREAD_CMD_EXIT );

    if ( cmd == THREAD_CMD_EXIT && conn ) {
        delete conn;
    }

#ifdef USE_PTHREAD
    return 0;
#endif // USE_PTHREAD
}

void ThreadMgr::assignTask ( Connection *conn ) {
    int temp, i;
    int least = 32768;
    int leastIndex = 0;

    if ( !conn )
    { return; }

    for ( i = 0; i < MAX_THREADS; i++ ) {
        temp = cmdPipe[i]->getUsage();

        //printf("Thread %d Usage=%d \n",i,temp);
        if ( temp < least ) {
            least = temp;
            leastIndex = i;
        }
    }

    //leastIndex = cIndex;
    //cIndex=(cIndex+1)%MAX_THREADS;
    //printf("Assigning Task to Thread=%d \n",leastIndex);
    cmdPipe[leastIndex]->pushCmd ( conn );
}
