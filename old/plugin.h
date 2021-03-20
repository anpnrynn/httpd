#ifndef HTTPD_PLUGINS_H
#define HTTPD_PLUGINS_H

#include <prlink.h>
#include <httphandlers.h>
#include <sqlite3.h>

typedef int( *INITFUNC) ();
typedef int( *EXITFUNC) ();
typedef int( *PROCFUNC) (Connection *);

struct qcmdinfo {
    int   qid;
    int   qtype;
    int   pnum;
    int   key;
    char  qcmd[256];
};

int   getqcmd          ( sqlite3 *db, int qid, int power, struct qcmdinfo *qcmd);
void  registerPlugin   ( PluginHandler* );
void  deregisterPlugin ( PluginHandler* );
int   sqlexecute ( sqlite3 *db, const char *sql, int (*callback)(void*,int,char**,char**), void *data , struct qcmdinfo *qcmd );
int   sqlhandler       ( void *udata, int argc, char **argv, char **colName);
int   sqlhandlertype2  ( void *udata, int argc, char **argv, char **colName);
int   sqlhandlertype3  ( void *udata, int argc, char **argv, char **colName);
int   appendrespstring ( Connection *conn, char *buf );
int   flushdata        ( Connection *conn, char **start, char **clen );
int   xmlstart         ( Connection *, int, int , int, char*);
int   xmlend           ( Connection *, int, char * );
int   xmlrsetstart     ( Connection *conn );
int   xmlrsetend       ( Connection *conn );
int   xmllastrowid     ( Connection *conn , int qid, unsigned long long key);

#endif
