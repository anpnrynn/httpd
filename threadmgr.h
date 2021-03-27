#ifndef THREADMGR_H
#define THREADMGR_H

#include <iostream>
#include <queue>
#include <thread>
#include <chrono>
#include <mutex>
using namespace std;
#include <pthread.h>
#include <time.h>
#include <httpconn.h>
#include <tools.h>

using namespace std;

#define MAX_POSSIBLE_THREADS 30
//#define MAX_THREADS          3

enum  ThreadCmd {
    THREAD_CMD_SLEEP = 0,
    THREAD_CMD_PTASK,
    THREAD_CMD_EXIT,
    THREAD_MAX_CMDS
};

typedef queue  <Connection *> CmdQueue;
class CmdPipe {
    private:
        mutex lock;
        CmdQueue *cmdQueue;
        int  usage;
        int  cmdQueueId;

    public:
        CmdPipe ( int id );
        ~CmdPipe();

        int  getUsage();
        void pushCmd ( Connection * );
        Connection * popCmd  ();
        void reduceUsage();
};

int sqlhandler ( void *, int, char **, char** );

class ThreadMgr {
    private:
        CmdPipe  *cmdPipe[MAX_POSSIBLE_THREADS];
        int       index  [MAX_POSSIBLE_THREADS];
#ifdef  USE_CPP11THREAD
        std::thread *tdata[MAX_POSSIBLE_THREADS];
#else
#ifndef USE_PTHREAD
        PRThread *tdata  [MAX_POSSIBLE_THREADS];
#else
        pthread_t tdata [MAX_POSSIBLE_THREADS];
#endif
#endif
        sqlite3  *db     [MAX_POSSIBLE_THREADS];
        int       cIndex;

        static ThreadMgr *tMgr;
        ThreadMgr();
    public:
        static  ThreadMgr* createInstance();
        void       startThreads();
        void       stopThreads();

        ~ThreadMgr();
#ifdef  USE_CPP11THREAD
        static void  httpthread ( int * );
#else
#ifndef USE_PTHREAD
        static void  httpthread ( void * );
#else
        static void*  httpthread ( void * );
#endif
#endif
        void assignTask    ( Connection * );
};


#endif
