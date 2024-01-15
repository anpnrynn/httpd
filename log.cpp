//Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3
#include <stdio.h>
#include <log.h>
#include <stdarg.h>

#define LOGBUFSIZE 8192
__thread char logbuf[LOGBUFSIZE];
int loglevel = 1;

void debuglog( const char *format, ... ){
	va_list ap;
	va_start(ap, format);
	int p = 0;
	
	switch( format[0] ){
		case 'E':
				if( loglevel >= 1 )
					p = 1;
				break;	
		case 'W':
				if( loglevel >= 2 )
					p = 1;
				break;	
		case 'I':
				if( loglevel >= 3 )
					p = 1;
				break;	
		case 'D':
				if( loglevel >= 4 )
					p = 1;
				break;	
		case 'X':
				if( loglevel >= 5 )
					p = 1;
				break;	
		default:
				if( loglevel >= 6 )
					p = 1;
				break;	
	}
	if( p == 1 ) {
		vsnprintf( logbuf, LOGBUFSIZE ,format, ap );
		//fprintf( stderr, logbuf );
		fputs( logbuf, stderr );
	}
	va_end(ap);
	return;
}
