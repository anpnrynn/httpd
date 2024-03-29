//Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3
#include <stdio.h>
#include <log.h>
#include <stdarg.h>

#define LOGBUFSIZE 8192
__thread char logbuf[LOGBUFSIZE];
int loglevel = 1;
int logdump  = 0;

void debuglog ( const char *format, ... ) {
    va_list ap;
    va_start ( ap, format );
    int p = 0;

    switch ( format[0] ) {
        case ' ':
            p = 1;
            break;

        case 'E':
            if ( loglevel >= 1 )
            { p = 1; }

            break;

        case 'W':
            if ( loglevel >= 2 )
            { p = 1; }

            break;

        case 'I':
            if ( loglevel >= 3 )
            { p = 1; }

            break;

        case 'D':
            if ( loglevel >= 4 )
            { p = 1; }

            break;

        case 'X':
            if ( logdump )
            { p = 1; }

            break;

        default:
            if ( loglevel >= 5 )
            { p = 1; }

            break;
    }

    if ( p == 1 ) {
        vsnprintf ( logbuf, LOGBUFSIZE - 1, format, ap );
        logbuf[LOGBUFSIZE - 1] = 0;
        fputs ( logbuf, stderr );
    }

    va_end ( ap );
    return;
}
