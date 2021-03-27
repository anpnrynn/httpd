#include <stdio.h>
#include <prwrapper.h>
#include <session.h>
//#include <openssl/ripemd.h>
#include <pass.h>
#include <sqlite3.h>
#include <defines.h>
//#include <sfeac.h>
#include <tools.h>

#ifndef LINUX_BUILD
#define random rand
#define srandom srand
#endif // LINUX_BUILD

#include <chrono>
#include <thread>
using namespace std;


HttpSession:: HttpSession ( unsigned int i, unsigned int addr, time_t e ) {
    sVars = new MapVarSess();
    /*
     *  Create the session id here :)
     */
    id     = i;
    ipaddr = addr;
    exp    = e;
    char buf[64];
    char sessionId[128];
	memset( buf, 0, 64 );
	memset( sessionId, 0, 64 );

	uint32_t randr = (uint32_t)i;
	uint32_t *randoms = (uint32_t*)buf;

	randoms[0] = rand_reentrant( &randr );
	randoms[1] = rand_reentrant( &randr );
	randoms[2] = rand_reentrant( &randr );
	randoms[3] = rand_reentrant( &randr );
	randoms[4] = rand_reentrant( &randr );
	randoms[5] = rand_reentrant( &randr );
	randoms[6] = rand_reentrant( &randr );
	randoms[7] = rand_reentrant( &randr );

    convertRandomsToHex( buf, sessionId );
    sid = sessionId;
    //fprintf ( stderr, "INFO: SessionId for id=%d randoms=%08u %08u %08u %08u %08u %08u %08u %08u \n",
    //    id, randoms[0], randoms[1], randoms[2], randoms[3], randoms[4], randoms[5], randoms[6], randoms[7] );
    fprintf ( stderr, "INFO: SessionId for id=%d sessionId='%s'\n", id, sessionId );
}

HttpSession::~HttpSession() {
    if ( sVars )
    { delete sVars; }
}

void HttpSession::loadVariable ( const char* name, char* value, bool i, time_t exp ) {
    MapVarSess::iterator itr = sVars->find ( name );

    if ( itr != sVars->end() ) {
        SessionVar *temp = itr->second;
        temp->setValues ( name, value, i, exp );
        return ;
    }

    SessionVar *s = new SessionVar ( name, value, i, exp );
    sVars->insert ( pair<string, SessionVar*> ( name, s ) );
}


void HttpSession::addVariable ( sqlite3 *db, const char* name, char* value, bool i, time_t exp ) {
    MapVarSess::iterator itr = sVars->find ( name );

    if ( itr != sVars->end() ) {
        SessionVar *temp = itr->second;
        temp->setValues ( name, value, i, exp );
        updateSessionVariable ( db, name );
        return ;
    }

    SessionVar *s = new SessionVar ( name, value, i, exp );
    sVars->insert ( pair<string, SessionVar*> ( name, s ) );

    saveSessionVariable ( db, name );
}

void HttpSession::delVariable ( sqlite3 *db, const char *name ) {
    MapVarSess::iterator i = sVars->find ( name );

    if ( i != sVars->end() ) {
        deleteSessionVariable ( db, name );

        if ( i->second )
        { delete i->second; }

        sVars->erase ( i );
    }
}

void HttpSession::setVariable ( sqlite3 *db, const char* name, char* value, bool isC, time_t exp ) {
    MapVarSess::iterator i = sVars->find ( name );

    if ( i != sVars->end() ) {
        i->second->setValues ( name, value, isC, exp );
        updateSessionVariable ( db, name );
    }
}

char* HttpSession::getVariable ( const char *name ) {
    static const char *empty = "";
    MapVarSess::iterator i = sVars->find ( name );

    if ( i != sVars->end() )
    { return i->second->value; }
    else
    { return ( char * ) empty; }
}

SessionVar* HttpSession::getVariableClass ( const char *name ) {
    //static const char *empty = "";
    MapVarSess::iterator i = sVars->find ( name );

    if ( i != sVars->end() )
    { return i->second; }
    else
    { return 0; }
}

void HttpSession::clear() {
    MapVarSess::iterator i = sVars->begin();

    while ( i != sVars->end() ) {
        if ( i->second )
        { delete i->second; }

        i++;
    }

    sVars->clear();
}

void HttpSession::expireAllCookies() {
    MapVarSess::iterator i = sVars->begin();
    time_t expiryDate;
    //LL_MUL ( expiryDate, 14400, 1000000);
    expiryDate = ( time_t ) ( ( ( time_t ) 14400 ) * ( ( time_t ) 1000000 ) );

    while ( i != sVars->end() ) {
        if ( i->second ) {
            i->second->expires = expiryDate;
        }

        i++;
    }
}

int HttpSession::dumpSessionCookies ( char *buf ) {
    MapVarSess::iterator i = sVars->begin();
    int bytesW = 0;
    struct tm temp;
    bytesW = sprintf ( buf, "Set-Cookie: SID=%s; ", sid.c_str() );

    if ( exp ) {
        gmtime_r ( ( const time_t * ) & exp,  &temp );
        bytesW += addDate ( &buf[bytesW], "expires", temp );
    }

    bytesW += sprintf ( &buf[bytesW], "\r\n" );

    while ( i != sVars->end() ) {
        if ( i->second && i->second->isCookie == 1 ) {
            bytesW += sprintf ( buf, "Set-Cookie: %s=%s; ", i->second->name, i->second->value );

            if ( exp ) {
                gmtime_r ( ( const time_t * ) & i->second->expires,  &temp );
                bytesW += addDate ( &buf[bytesW], "expires", temp );
            }

            bytesW += sprintf ( &buf[bytesW], "\r\n" );
        }

        i++;
    }

    return bytesW;
}

MapStrStr* HttpSession::readCookies ( char *buf ) {
    int i = 0;

    if ( buf[0] == 0 )
    { return NULL; }

    int j = 0;
    char  name[256];
    char value[256];
    char *data = name;
    MapStrStr *cookies = new MapStrStr();

    while ( ( buf[i] != '\0' )  && ( i < 256 ) ) {
        switch ( buf[i] ) {
            case ';':
                data[j] = 0;
                cookies->insert ( pair<string, string> ( name, value ) );
                data = name;
                j = 0;
                break;

            case '=':
                data[j] = 0;
                data = value;
                j = 0;
                break;

            case ' ':
                if ( j > 0 )
                { data[j++] = ' '; }

                break;

            default :
                data[j++] = buf[i];
                break;
        }

        i++;
    }

    data[j] = 0;
    cookies->insert ( pair<string, string> ( name, value ) );
    return cookies;
}

int HttpSession::saveSession ( sqlite3 *db ) {
    if ( !db )
    { return 1; }

    char *error = NULL;
    char  buf[256];
    int   rc = 0;
    MapVarSess::iterator itr = sVars->begin();
    SessionVar *sVar = NULL;

    while ( itr != sVars->end() ) {
        sVar = itr->second ;

        if ( sVar && sVar->name && sVar->value )
        { sprintf ( buf, "insert into sessions values(%d, %d, %d, %d, '%s', '%s');", id, ipaddr, ( unsigned int ) exp, sVar->isCookie, sVar->name, sVar->value ); }
        else
        { sprintf ( buf, "insert into sessions ( id, ipaddr, eos)values(%d, %d, %d);", id, ipaddr, ( unsigned int ) exp ); }

        do {
            rc = sqlite3_exec ( db, buf, NULL, NULL, &error );

            if ( rc != SQLITE_OK ) {
                if ( rc == SQLITE_BUSY )
                { std::this_thread::sleep_for(std::chrono::microseconds(10)); /*PR_Sleep ( 10 );*/ }
                else {
                    fprintf(stderr, "ERRR: Unable to store session while saving '%s'\n", error );

                    if ( error )
                    { sqlite3_free ( error ); }

                    return 2;
                }
            }
        } while ( rc == SQLITE_BUSY );

        itr++;
    }

    return 0;
}

int HttpSession::saveSessionVariable ( sqlite3 *db, const char *name ) {
    if ( !db )
    { return 1; }

    char *error = NULL;
    char  buf[256];
    int   rc = 0;
    //MapVarSess::iterator itr = sVars->find( name );
    SessionVar *sVar = getVariableClass ( name ) ;

    if ( sVar ) {
        if ( sVar && sVar->name && sVar->value ) {
            sprintf ( buf, "insert into sessions values(%d, %d, %d, %d, '%s', '%s');", id, ipaddr, ( unsigned int ) exp, sVar->isCookie, sVar->name, sVar->value );

            do {
                rc = sqlite3_exec ( db, buf, NULL, NULL, &error );

                if ( rc == SQLITE_OK )
                { break; }
                else if ( rc == SQLITE_BUSY )
                { std::this_thread::sleep_for(std::chrono::microseconds(10)); /*PR_Sleep ( 10 );*/ }
                else {
                    fprintf(stderr, "ERRR: Unable to store session variable: %s,  '%s'\n", name, error );

                    if ( error )
                    { sqlite3_free ( error ); }

                    return 2;
                }
            } while ( rc != SQLITE_OK );
        }
    }

    return 0;
}


int HttpSession::updateSaveSession ( sqlite3 *db ) {
    if ( !db )
    { return 1; }

    char *error = NULL;
    char  buf[256];
    int   rc = 0;
    MapVarSess::iterator itr = sVars->begin();
    SessionVar *sVar = NULL;

    while ( itr != sVars->end() ) {
        sVar = itr->second ;

        if ( sVar && sVar->name && sVar->value ) {
            sprintf ( buf, "update sessions set value='%s' where id=%d and ipaddr=%d and eos=%d and name='%s';",  sVar->value, id, ipaddr, ( unsigned int ) exp, sVar->name );

            do {
                rc = sqlite3_exec ( db, buf, NULL, NULL, &error );

                if ( rc != SQLITE_OK ) {
                    if ( rc == SQLITE_BUSY )
                    { std::this_thread::sleep_for(std::chrono::microseconds(10)); /*PR_Sleep ( 10 );*/ }
                    else {
                        fprintf(stderr, "ERRR: Unable to store session while updating '%s'\n", error );

                        if ( error )
                        { sqlite3_free ( error ); }

                        return 2;
                    }
                }
            } while ( rc == SQLITE_BUSY );
        }

        itr++;
    }

    return 0;
}

int HttpSession::updateSessionVariable ( sqlite3 *db, const char *name ) {
    if ( !db )
    { return 1; }

    char *error = NULL;
    char  buf[256];
    int   rc = 0;
    //MapVarSess::iterator itr = sVars->begin();
    SessionVar *sVar = getVariableClass ( name );

    if ( sVar ) {
        if ( sVar && sVar->name && sVar->value ) {
            sprintf ( buf, "update sessions set value='%s' where id=%d and ipaddr=%d and eos=%d and name='%s';",  sVar->value, id, ipaddr, ( unsigned int ) exp, sVar->name );

            //do
            //{
            do {
                rc = sqlite3_exec ( db, buf, NULL, NULL, &error );

                if ( rc == SQLITE_OK )
                { break; }
                else if ( rc == SQLITE_BUSY )
                { std::this_thread::sleep_for(std::chrono::microseconds(10)); /*PR_Sleep ( 10 );*/ }
                else {
                    fprintf(stderr, "ERRR: Unable to update session variable: %s, '%s'\n", sVar->name, error );

                    if ( error )
                    { sqlite3_free ( error ); }

                    return 2;
                }
            } while ( rc != SQLITE_OK );

            //}while( rc == SQLITE_BUSY );
        }
    }

    return 0;
}

int HttpSession::deleteSessionVariable ( sqlite3 *db, const char *name ) {
    if ( !db )
    { return 1; }

    char *error = NULL;
    char  buf[256];
    int   rc = 0;
    //MapVarSess::iterator itr = sVars->begin();
    SessionVar *sVar = getVariableClass ( name );

    if ( sVar ) {
        if ( sVar && sVar->name && sVar->value ) {
            sprintf ( buf, "delete from sessions where id=%d and ipaddr=%d and eos=%d and name='%s';", id, ipaddr, ( unsigned int ) exp, sVar->name );

            //do
            //{
            do {
                rc = sqlite3_exec ( db, buf, NULL, NULL, &error );

                if ( rc == SQLITE_OK )
                { break; }
                else if ( rc == SQLITE_BUSY )
                { std::this_thread::sleep_for(std::chrono::microseconds(10)); /*PR_Sleep ( 10 );*/ }
                else {
                    fprintf(stderr, "ERRR: Unable to update session variable: %s, '%s'\n", sVar->name, error );

                    if ( error )
                    { sqlite3_free ( error ); }

                    return 2;
                }
            } while ( rc != SQLITE_OK );

            //}while( rc == SQLITE_BUSY );
        }
    }

    return 0;
}



HttpSessionMgr* HttpSessionMgr::hsmgr;

HttpSessionMgr::HttpSessionMgr() {
    mapSid = new MapSidSess();
    time_t temp = time ( NULL );
    srand ( ( unsigned int ) temp );
    randomNum = 0;
}

HttpSessionMgr::~HttpSessionMgr() {
    MapSidSess::iterator i = mapSid->begin();

    while ( i != mapSid->end () ) {
        if ( i->second )
        { delete i->second; }

        i++;
    }

    mapSid->clear();
}

HttpSessionMgr* HttpSessionMgr::createInstance() {
    if ( !hsmgr ) {
        hsmgr = new HttpSessionMgr();
    }

    return hsmgr;
}

HttpSession* HttpSessionMgr::loadSession ( unsigned int rand, unsigned int ipaddr, time_t expires ) {
    HttpSession *temp = new HttpSession ( rand, ipaddr, expires );

    if ( temp ) {
        //map<string,HttpSession*>::iterator itr1 = mapSid->find ( temp->sid );
        MapSidSess::iterator itr1 = mapSid->find ( temp->sid );

        if ( itr1 != mapSid->end() ) {
            fprintf(stderr, "INFO: Retrieved session '%s': %d %x %u \n", temp->sid.c_str(), rand, ipaddr, ( unsigned int ) expires );
            delete temp;
            temp = itr1->second;
        } else {
            fprintf(stderr, "INFO: Adding Session '%s': %d %x %u \n", temp->sid.c_str(), rand, ipaddr, ( unsigned int ) expires );
            mapSid->insert ( pair<string, HttpSession*> ( temp->sid, temp ) );
        }
    }

    return temp;
}

HttpSession* HttpSessionMgr::startSession ( unsigned int ipaddr, time_t expires ) {
    /*
     *  Get the last used id value
     */

    sqlite3 *db = NULL;
    int   rc = 0;
    //rc = sqlite3_open(INFO_STORE, &db, getEncryptionStructure( ENC_KEY_DATABASE ) );
    rc = sqlite3_open ( INFO_STORE, &db );

    if ( rc ) {
        fprintf(stderr, "ERRR: Unable to open db, Corrupted ? \n" );
        sqlite3_close ( db );
        db = NULL;
        return 0;
    }

    if ( randomNum == 0 ) {
        time_t temp = time ( NULL );
        randomNum   = rand() ^ temp;
    } else {
		randomNum   = rand();
	}

    //randomNum++;

    HttpSession *temp = new HttpSession ( randomNum, ipaddr, expires );
    temp->addVariable ( db, "auth",      ( char * ) "1", 0, 0 );
    temp->addVariable ( db, "friendname", ( char * ) "0", 0, 0 );
    temp->addVariable ( db, "isfriend",  ( char * ) "0", 0, 0 );
    temp->addVariable ( db, "userlevel", ( char * ) "0", 0, 0 );
    temp->addVariable ( db, "loggedin",  ( char * ) "0", 0, 0 );
    temp->addVariable ( db, "localuser", ( char * ) "0", 0, 0 );
    temp->addVariable ( db, "random",    ( char * ) "0", 0, 0 );
    //TODO: remove saveSession , now addVariable
    //temp->saveSession (db);
    sqlite3_close ( db );

    if ( temp != NULL )
    { mapSid->insert ( pair<string, HttpSession*> ( temp->sid, temp ) ); }

    return temp;
}


HttpSession* HttpSessionMgr::getSession ( unsigned int ipaddr, string sid ) {
    MapSidSess::iterator i = mapSid->find ( sid );

    if ( i != mapSid->end( ) ) {
        if ( i->second && i->second->ipaddr == ipaddr )
        { return i->second; }
    }

    return NULL;
}

void HttpSessionMgr::endSession ( string sid ) {
    MapSidSess::iterator i = mapSid->find ( sid );

    if ( i != mapSid->end() ) {
        if ( i->second )
        { delete i->second; }

        mapSid->erase ( i );
    }
}

int  HttpSessionMgr::loadStoredSessions() {
    sqlite3 *db = NULL;
    int   rc = 0;
    char *error = NULL;
    //rc = sqlite3_open(INFO_STORE, &db, getEncryptionStructure( ENC_KEY_DATABASE ) );
    rc = sqlite3_open ( INFO_STORE, &db );

    if ( rc ) {
        fprintf(stderr, " ERRR: Unable to open db, Corrupted ? \n" );
        sqlite3_close ( db );
        db = NULL;
        return 1;
    }

    time_t today = time ( NULL );
    char tbuf[128];
    sprintf ( tbuf, "delete from sessions where eos < %d;", ( unsigned int ) ( today - 86400 ) );
    rc = sqlite3_exec ( db, tbuf, NULL, NULL, &error );

    if ( rc != SQLITE_OK ) {
        if ( error ) {
            sqlite3_free ( error );
        }

        //printf("ERRR: Unable to read session data\n");
    }

    rc = sqlite3_exec ( db, "select * from sessions;", readSessionInfo, this, &error );

    if ( rc != SQLITE_OK ) {
        if ( error ) {
            sqlite3_free ( error );
        }

        //printf("ERRR: Unable to read session data\n");
        //sqlite3_close(db);
        //return 2;
    }

    sqlite3_close ( db );
    return 0;
}

int  HttpSessionMgr::deleteStoredSessions() {
    sqlite3 *db = NULL;
    int   rc = 0;
    char *error = NULL;
    //rc = sqlite3_open(INFO_STORE, &db, getEncryptionStructure( ENC_KEY_DATABASE ) );
    rc = sqlite3_open ( INFO_STORE, &db );

    if ( rc ) {
        fprintf(stderr, " ERRR: Unable to open db, Corrupted ? \n" );
        sqlite3_close ( db );
        db = NULL;
        return 1;
    }

    time_t today = time ( NULL );
    char tbuf[128];
    sprintf ( tbuf, "delete from sessions where eos < %d;", ( unsigned int ) ( today - 86400 ) );
    rc = sqlite3_exec ( db, tbuf, NULL, NULL, &error );

    if ( rc != SQLITE_OK ) {
        if ( error ) {
            sqlite3_free ( error );
        }

        //printf("ERRR: Unable to read session data\n");
    }

    sqlite3_close ( db );
    return 0;
}

int  HttpSessionMgr::readSessionInfo ( void *udata, int argc, char **argv, char **cName ) {
    HttpSessionMgr *hTemp = ( HttpSessionMgr * ) udata;
    fprintf(stderr, "INFO: Reading Session Data \n" );

    if ( argv[0] && argv[1] && argv[2] ) {
        HttpSession *temp = hTemp ->loadSession ( atoi ( argv[0] ), atoi ( argv[1] ), atoi ( argv[2] ) );

        if ( argv[3] && argv[4] && argv[5] ) {
            fprintf(stderr, "INFO: Adding Variable %s %s %s\n", argv[3], argv[4], argv[5] );
            temp->loadVariable ( ( const char * ) argv[4], argv[5], atoi ( argv[3] ), atoi ( argv[2] ) );
        }
    }

    return 0;
}

int HttpSessionMgr::saveSession() {
    sqlite3 *db = NULL;
    int   rc = 0;
    //rc = sqlite3_open(INFO_STORE, &db, getEncryptionStructure( ENC_KEY_DATABASE ) );
    rc = sqlite3_open ( INFO_STORE, &db );

    if ( rc ) {
        fprintf(stderr, "ERRR: Unable to open db, Corrupted ? \n" );
        sqlite3_close ( db );
        db = NULL;
        return 1;
    }

    MapSidSess::iterator i = mapSid->begin();
    HttpSession *temp = NULL;

    while ( i != mapSid->end() ) {
        temp = i->second;
        temp->saveSession ( db );
        i++;
    }

    sqlite3_close ( db );
    return 0;
}
