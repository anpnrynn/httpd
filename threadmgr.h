#ifndef THREADMGR_H
#define THREADMGR_H

#include <iostream>
#include <queue>
#ifndef  USE_PTHREAD
#include <prthread.h>
#include <prlock.h>
#else
#include <pthread.h>
#include <time.h>
#endif
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
#ifndef USE_PTHREAD
        PRLock   *lock;
#else
        pthread_mutex_t lock;
#endif
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
#ifndef USE_PTHREAD
        PRThread *tdata  [MAX_POSSIBLE_THREADS];
#else
        pthread_t tdata [MAX_POSSIBLE_THREADS];
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
#ifndef USE_PTHREAD
        static void  thread ( void * );
#else
        static void*  thread ( void * );
#endif
        void assignTask    ( Connection * );
};


#endif
