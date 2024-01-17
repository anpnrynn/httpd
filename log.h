//Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3
#ifndef HTTPD_LOG_H
#define HTTPD_LOG_H

void debuglog ( const char *format, ... );
extern int loglevel;
extern int logdump;

#endif
