//Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3
#ifndef HTTP_SESSION_H
#define HTTP_SESSION_H

#include <tools.h>
//#include <prlong.h>
#include <ctime>
#include <apptypes.h>
#include <cookie.h>
using namespace std;
#include <string.h>
#include <stdlib.h>
#include <log.h>

#define  SESSVAR_BUFSIZE 256
#define  SESSVAR_EXPIRED 3600000000

class SessionVar {
    public:
        char  name [SESSVAR_BUFSIZE];
        char  value[SESSVAR_BUFSIZE];
        short int isCookie;
        time_t    tdiff;
        time_t    expires;

        SessionVar() {
            name[0]  = 0;
            value[0] = 0;
            isCookie = 0;
            tdiff    = 0;
            expires  = 0;
        }

        SessionVar ( const char* n, char *v, short int i, time_t e ) {
            strcpy ( name, n );
            strcpy ( value, v );
            isCookie = i;
            time_t temp = time ( NULL );
            expires  = e;
            //LL_SUB( tdiff, expires, temp);
            tdiff = expires - temp;
        }

        void setValues ( const char* n, char *v, short int i, time_t e ) {
            strcpy ( name, n );
            strcpy ( value, v );
            isCookie = i;
            time_t temp = time ( NULL );
            expires  = e;
            //LL_SUB( tdiff, expires, temp);
            tdiff = expires - temp;
        }

        void resetExpiry() {
            time_t temp = time ( NULL );
            //LL_ADD( expires, temp, tdiff );
            expires = temp + tdiff;
        }

        ~SessionVar() {}
};

#ifndef LINUX_BUILD
typedef map<string, SessionVar *> MapVarSess;
#else
typedef unordered_map<string, SessionVar *> MapVarSess;
#endif

class HttpSession {
    private:

    protected:
        MapVarSess   *sVars;

    public:
        unsigned int   id;
        string        sid;
        time_t        exp;
        unsigned int  ipaddr;
        HttpSession ( unsigned int, unsigned int, time_t );
        ~HttpSession();

        //HttpSession(const HttpSession &);
        void   loadVariable  ( const char *, char *, bool, time_t );
        char*  getVariable   ( const char * );

        void   addVariable   ( sqlite3 *db, const char *, char *, bool, time_t );
        void   delVariable   ( sqlite3 *db, const char * );
        void   setVariable   ( sqlite3 *db, const char *, char *, bool, time_t );
        void   clear();
        void   expireAllCookies();
        int    dumpSessionCookies ( char *buf );
        static MapStrStr* readCookies ( char *buf );

        int    saveSession ( sqlite3 * );
        int    updateSaveSession ( sqlite3 *db );

    private:
        SessionVar*  getVariableClass   ( const char * );
        int    saveSessionVariable ( sqlite3 *, const char * );
        int    updateSessionVariable ( sqlite3 *db, const char * );
        int    deleteSessionVariable ( sqlite3 *db, const char * );
};

#ifndef LINUX_BUILD
typedef map<string, HttpSession*> MapSidSess;
#else
typedef unordered_map<string, HttpSession*> MapSidSess;
#endif
class HttpSessionMgr {
    private:
        MapSidSess *mapSid;
        HttpSessionMgr();
        static HttpSessionMgr* hsmgr;
        unsigned int    randomNum;

    public:
        static HttpSessionMgr* createInstance();

        ~HttpSessionMgr();
        HttpSession * getSession      ( unsigned int, string );
        HttpSession * startSession    ( unsigned int, time_t );
        HttpSession * loadSession     ( unsigned int, unsigned int, time_t );
        void          endSession      ( string );

        int           saveSession     ();
        int           loadStoredSessions ();
        int           deleteStoredSessions ();
        static int    readSessionInfo ( void*, int, char**, char ** );
};

#endif
