//Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3
#ifndef ___DEFINES__H___
#define ___DEFINES__H___

#include <config.h>

#ifdef  LINUX_BUILD
#ifdef  CODEBLOCKS_BUILD
#define HSERVER_HOME "."
#define INSTALL_HOME HSERVER_HOME
#define CERT_STORE  HSERVER_HOME "/share"
#define INFO_STORE  HSERVER_HOME "/var/local/infostore.sqlite3"
#define PAGE_STORE  HSERVER_HOME "/var/www/Pages/"
#define LOGFILE     HSERVER_HOME "/var/githubcom_anpnrynn_httpdsrvwin.log"
#else
#define SERVER_HOME INSTALL_HOME "/httpd/"
#define CERT_STORE  INSTALL_HOME "/httpd/share/"
#define INFO_STORE  "/var/local/infostore.sqlite3"
#define PAGE_STORE  INSTALL_PAGE_STORE "/var/www/Pages/"
#define LOGFILE     "/var/log/githubcom_anpnrynn_httpd.log"
#endif
#else
#define SERVER_HOME INSTALL_HOME "/httpd/"
#define CERT_STORE  INSTALL_HOME "/httpd/share/"
#define INFO_STORE  "infostore.sqlite3"
#define PAGE_STORE  INSTALL_PAGE_STORE "Pages/"
#define LOGFILE     "githubcom_anpnrynn_httpd.log"
#endif

#define MAXPATH     256
//#define MAXBUF      16384
//#define SRVPORT     3333
#define CONST_MAXCLIENTS  1024 // <= 10 threads
#define MAXPLUGINS  64


#endif
