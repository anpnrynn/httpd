#ifndef ___DEFINES__H___
#define ___DEFINES__H___


#ifdef  LINUX_BUILD
#define FILE_STORE  "/var/local/filestore"
#define INFO_STORE  "/var/local/infostore.sqlite3"
#define KEYFILE     "/var/local/db.key"
#else
#define FILE_STORE  "filestore"
#define INFO_STORE  "infostore"
#define KEYFILE     "db.key"
#endif

#define MAXPATH     256
//#define MAXBUF      16384
//#define SRVPORT     3333
#define MAXCLIENTS  1024
#define MAXPLUGINS  64


#endif
