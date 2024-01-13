#ifndef ___DEFINES__H___
#define ___DEFINES__H___

#include <config.h>

#ifdef  LINUX_BUILD
#define INFO_STORE  "/var/local/infostore.sqlite3"
#define PAGE_STORE  INSTALL_PAGE_STORE "/var/www/Pages/" 
#define LOGFILE     "/var/log/githubcom_anpnrynn_httpd.log"
#else
#define INFO_STORE  "infostore"
#define PAGE_STORE  INSTALL_PAGE_STORE "Pages/" 
#define LOGFILE     "githubcom_anpnrynn_httpd.log"
#endif

#define MAXPATH     256
//#define MAXBUF      16384
//#define SRVPORT     3333
#define MAXCLIENTS  1024
#define MAXPLUGINS  64


#endif
