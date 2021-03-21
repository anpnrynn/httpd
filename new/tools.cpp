#include <tools.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <strings.h>

#ifndef LINUX_BUILD
#include <string.h>
//#define strncasecmp _strnicmp
#endif // LINUX_BUILD

void httpencode( char *to, char *from, int len )
{
	int tolen;
	for ( tolen = 0; *from != '\0' && tolen + 4 < len; ++from )
	{
		if ( isalnum(*from) || strchr( "/_.-~", *from ) != (char*) 0 )
		{
			*to = *from;
			++to;
			++tolen;
		}
		else
		{
			(void) sprintf( to, "%%%02x", (int) *from & 0xff );
			to += 3;
			tolen += 3;
		}
	}
	*to = '\0';
}

char hexCharToHexBin(char c )
{
	if( c >= '0' && c<= '9')
		return (c-'0');
	else
	if( c >= 'a' && c <= 'f')
		return (c-'a'+10);
	else
	if( c >= 'A' && c <= 'F')
		return (c-'A'+10);
	else
		return (char)0;
}

void httpdecode( char *to, char * from)
{
	for( ; *from != '\0'; ++to,++from )
	{
		if( *from == '%' && isxdigit(from[1]) && isxdigit(from[2]) )
		{
			*to = hexCharToHexBin(from[1]) *16 + hexCharToHexBin(from[2]);
			from+=2;
		}
		else
		{
			*to = *from;
		}
	}
	*to = '\0';
}


int httpdecodepartial( char *to, char * from, int len)
{
    int i=0;
    char *start=from;
	for( i=0; i<len; ++i, ++to, ++from)
	{
        if( *from == '%' )
        {
            if( i <= len-3 )
            {
                if( isxdigit(from[1]) && isxdigit(from[2]) )
		        {
		  	       *to = hexCharToHexBin(from[1]) *16 + hexCharToHexBin(from[2]);
			        from+=2;
			        i+=2;
                }
            }
            else
            {
                start[0] = *from;
                start[1] = from[1];
                return 2;
            }
        }
        else
        if( *from == '+' )
        {
            *to = ' ';
        }
        else
        {
            *to = *from;
        }
	}
	return 0;
}

const char* convertIndexToWeekDay( int wdayIndex )
{
	static const char *wDay[10] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	if( wdayIndex < 0 || wdayIndex >6 )
	{
		printf("Error: invalid weekday index \n");
		return wDay[0];
	}
	else
		return wDay[wdayIndex];
}

const char* convertIndexToMonth( int monthIndex )
{
	static const char *month[16] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	if( monthIndex < 0 || monthIndex > 11 )
	{
		printf("Error: invalid month index \n");
		return month[0];
	}
	else
		return month[monthIndex];
}

int convertWeekDayToIndex(char *wday )
{
	if( strncasecmp("sun", wday, 3) == 0 )
		return 0;
	else
	if( strncasecmp("mon", wday, 3) == 0 )
		return 1;
	else
	if( strncasecmp("tue", wday, 3) == 0 )
		return 2;
	else
	if( strncasecmp("wed", wday, 3) == 0 )
		return 3;
	else
	if( strncasecmp("thu", wday, 3) == 0 )
		return 4;
	else
	if( strncasecmp("fri", wday, 3) == 0 )
		return 5;
	else
	if( strncasecmp("sat", wday, 3) == 0 )
		return 6;
	else
	{
		printf("Error: No matching weekday found in the date string\n");
		return -1;
	}
}

int convertMonthToIndex( char *month )
{
	if( strncasecmp("jan", month, 3) == 0 )
		return 0;
	else
	if( strncasecmp("feb", month, 3) == 0 )
		return 1;
	else
	if( strncasecmp("mar", month, 3) == 0 )
		return 2;
	else
	if( strncasecmp("apr", month, 3) == 0 )
		return 3;
	else
	if( strncasecmp("may", month, 3) == 0 )
		return 4;
	else
	if( strncasecmp("jun", month, 3) == 0 )
		return 5;
	else
	if( strncasecmp("jul", month, 3) == 0 )
		return 6;
	else
	if( strncasecmp("aug", month, 3) == 0 )
		return 7;
	else
	if( strncasecmp("sep", month, 3) == 0 )
		return 8;
	else
	if( strncasecmp("oct", month, 3) == 0 )
		return 9;
	else
	if( strncasecmp("nov", month, 3) == 0 )
		return 10;
	else
	if( strncasecmp("dec", month, 3) == 0 )
		return 11;
	else
	{
		printf("Error: No matching month found in the date string\n");
		return -1;
	}
}

int convertHttpDateToNsprDate(char *date, struct tm *eDate)
{
	char tempWday[32];
	char tempMonth[32];
	int  hWDay;
	int  hMonth;
	int  hDate;
	int  hYear;
	int  hHour;
	int  hMin;
	int  hSec;
	int  proceed = 0;

	if( !date || !eDate)
		return 0;
	for( ; *date == ' ' || *date == '\t'; date++ )
		continue;

	if( sscanf ( date, "%32[a-zA-Z], %d %32[a-zA-Z] %d %d:%d:%d GMT",
			tempWday, &hDate, tempMonth, &hYear, &hHour, &hMin, &hSec) == 7 )
	{
		proceed = 1;
	}
	else
	if( sscanf ( date, "%32[a-zA-Z], %d-%32[a-zA-Z]-%d %d:%d:%d GMT",
			tempWday, &hDate, tempMonth, &hYear, &hHour, &hMin, &hSec) == 7 )
	{
		proceed = 1;
	}
	else
	if( sscanf ( date, "%32[a-zA-Z] %32[a-zA-Z] %d %d:%d:%d %d GMT",
			tempWday, tempMonth, &hDate, &hHour, &hMin, &hSec, &hYear) == 7 )
	{
		proceed = 1;
	}
	else
	{
		printf("Error: No Matching Date format found \n");
		proceed = 0;
	}

	if( proceed )
	{
		if( (hWDay = convertWeekDayToIndex(tempWday)) == -1 )
			return 0;

		if( (hMonth = convertMonthToIndex(tempMonth)) == -1 )
			return 0;

		eDate->tm_year  = (uint16_t) hYear;
		eDate->tm_mon   = (int32_t) hMonth;
		eDate->tm_mday  = (int32_t) hDate;
		eDate->tm_hour  = (int32_t) hHour;
		eDate->tm_min   = (int32_t) hMin;
		eDate->tm_sec   = (int32_t) hSec;
		eDate->tm_wday  = (int8_t)  hWDay;
		return 1;
	}
	else
		return 0;
}

void convertMd5ToHexMd5( char *md5, char *hexmd5 )
{
	int i, j = 0;
	for( i=0; i<20; i++ )
	{
		j+= sprintf(&hexmd5[j],"%02x", (int)md5[i] & 0xff );
	}
	hexmd5[40] = 0;
}

int addDate( char *hdr, const char *field, struct tm &temp )
{
	return sprintf(hdr,"%s=%s, %s%d %s %4d %s%d:%s%d:%s%d GMT",
         field , convertIndexToWeekDay(temp.tm_wday), temp.tm_mday<10?"0":"",temp.tm_mday, convertIndexToMonth( temp.tm_mon),
         (1900+temp.tm_year), temp.tm_hour<10?"0":"", temp.tm_hour, temp.tm_min<10?"0":"", temp.tm_min, temp.tm_sec<10?"0":"", temp.tm_sec);
}





unsigned int createChunk(	unsigned char  *buffer,
							unsigned int   &buflen,
							unsigned char **start,
							unsigned char **clength)
{
	char temp[5];
	sprintf(temp,"%4x",(SMALLBUF-CHUNK_HDR_SIZE));
	*clength   = &buffer[2];
	*start     = &buffer[CHUNK_HDR_SIZE];
	buffer[0]  = '\r';
	buffer[1]  = '\n';
	buffer[6]  = '\r';
	buffer[7]  = '\n';
	memcpy (*clength,temp, 4);
	buflen     = CHUNK_HDR_SIZE;
	return (SMALLBUF-CHUNK_HDR_SIZE);
}

void  appendChunkData(  unsigned char  **start,
						unsigned int    &length,
						unsigned int    &rspace,
						unsigned char   *databuf,
						unsigned int     dlength ,
						unsigned char  **incompete )
{
	if( rspace >= dlength )
	{
		 memcpy( *start, databuf, dlength );
		 length  += dlength;
		*start   += dlength;
		 rspace  -= dlength; \
	} else {
		if( (rspace) > 0 ){
			memcpy( *start , databuf, rspace );
			*start    += rspace;
			 length   += rspace;
			*incompete = databuf+rspace;
			 rspace    = 0;
		}
	}
}


void appendChunkString( unsigned char **start,
						unsigned int   &length,
						unsigned int   &rspace,
						unsigned char  *databuf ,
						unsigned char **incompete)
{
	unsigned int i = 0;
	*incompete = NULL;
	while( *(databuf+i) != '\0' && i < rspace  )
	{
		*(*(start)+i) = *(databuf+i);
		i++;
	}
	rspace -= i;
	length += i;
	*start  += i;
	if( *(databuf+i) != '\0' ) {
		*incompete = (databuf+i);
	}
}

void appendChunkChar(   unsigned char **start,
						unsigned int   &length,
						unsigned int   &rspace,
						unsigned char   data )
{
	if( rspace >= 1 )
	{
		**(start) = data;
		 rspace--;
		 (*start)++;
		 length++;
	}
}

void truncateChunk( unsigned char  *clen,
					unsigned int   &len	)
{
	char temp[5];
	sprintf(temp,"%4x",len-CHUNK_HDR_SIZE);
	memcpy (clen, temp, 4);
}

void  lastChunk   ( unsigned char  *buf,
					unsigned int   &len )
{
	len += sprintf((char*)buf,"\r\n0\r\n\r\n");
}

void  readConfigFile ( const char *fileName, const char delim, void (*callback)(char *, char*) )
{

#if 0
	Ifs *ifs = NULL; //Ifs::createInstance();
	FileIndexData *fData;
	PRFileDesc *file = ifs->getFilePointer(fileName,&fData);
	unsigned char buf[256];
	char name[64];
	char value[64];
	char *data = name;
	int  i=0, j=0;
	int  len=0;
	int  offset=0;
	if( file )
	{
		do {
		len = ifs->ifsRead(file, fData, buf, 256, &offset);
		if( len > 0 )
		{
			for( i=0; i<len; i++ )
			{
				if(  buf[i] == delim ){
					data[j++] = 0;
					data = value;
					j=0;
				} else if ( buf[i] == '\r' ) {
				} else if ( buf[i] == '\n' ) {
					data[j++]=0;
					j=0;
					data = name;
					if( callback )
					{
						callback (name, value);
					}
				} else {
					data[j++] = buf[i];
				}
			}
		}
		}
		while( len > 0 );
		PR_Close(file);
	}
#endif
}

CSVReader :: CSVReader ( int col, void (*func) (char **, int , void *), void* dataPtr , char eChar)
{
    cNum = col;
    readData = func;
    data = dataPtr;
    pCol = 0;
    offset = 0;
    escape = 0;
    escapeChar = eChar;
}

int CSVReader ::parseData (char *buf, int len)
{
    int i;
    for( i=0; i<len; i++ )
    {
        if( buf[i] == '\n' )
        {
            if( offset )
            {
                if( escape )
                {
                    escape = 0;
                    cols[pCol][offset++] = '\\';
                }
                cols[pCol][offset] = '\0';
                offset=0;
            }
            if(readData)
            {
                readData( (char **)cols, pCol, data);
            }
        }
        else
        if( buf[i] == '\r' )
        {
            if( escape )
            {
                escape = 0;
                cols[pCol][offset++] = '\\';
            }
            cols[pCol][offset] = '\0';
            offset=0;
        }
        else
        if( buf[i] == ',' )
        {
            if( escape )
            {
                cols[pCol][offset++] = ',';
            }
            else
            {
                cols[pCol][offset] = '\0';
                offset=0;
                pCol++;
            }
        }
        else
        if( buf[i] == escapeChar )
        {
            escape=1;
        }
        else
        {
            if( escape )
            {
                escape = 0;
                cols[pCol][offset++] = '\\';
            }
            cols[pCol][offset++] =  buf[i];
        }
    }
return 0;
}


#ifdef OS_WIN

void gmtime_r( const time_t *date,  struct tm *temp )
{
 // I will need to replace this later...
 memcpy ( temp, gmtime( date ), sizeof( struct tm ) );
}

char* strtok_r( char *buf, char *delim, char **restart)
{
  if( *restart == NULL )
      *restart = buf;

  char *string = *restart;
  while( **restart != '\0' )
  {
         if( **restart == *delim )
         {
              **restart = '\0';
              (*restart)++;
              break;
         }
        (*restart)++;
  }
  return string;
}

#endif

int catstring ( char *result, char *data , int &rn)
{
    while ( *data != '\0' )
    {
        result[rn] = *data;
        data++;
        rn++;
    }
return 0;
}

int mergestring ( char *result, char *buf, char *data , int &rn, int &bn)
{
    while ( buf[bn] != '$' && buf[bn] != '\0' )
    {
        result[rn] = buf[bn];
        bn++;
        rn++;
    }

    if( *buf == '\0' )
        return 0;
    else
    {
        bn+=2;
        while ( *data != '\0' )
        {
            result[rn] = *data;
            data++;
            rn++;
        }
    }
    result[rn] =0;
return 0;
}

int mergeremaining ( char *result, char *buf )
{
    while ( *buf != '\0' )
    {
        *result = *buf;
        result++;
        buf++;
    }
    *result = 0;
    return 0;
}
