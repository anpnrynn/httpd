//Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3
#ifndef HTTP_TOOLS_H
#define HTTP_TOOLS_H

#include <sqlite3.h>
#include <apptypes.h>
//#include <prio.h>
//#include <prlong.h>
//#include <prerror.h>
//#include <prthread.h>
#include <ctime>
using namespace std;

#define CHUNK_HDR_SIZE 8

class ChunkHdr {
    public:
        unsigned char *start;
        unsigned char *clength;
        unsigned int  rspace;

        ChunkHdr() : start ( NULL ), clength ( NULL ), rspace ( SMALLBUF ) {}
        ~ChunkHdr() {
            start = NULL;
            clength  = NULL;
            rspace = 0;
        }
        void getValues ( unsigned char **s, unsigned char ** c, unsigned int *l ) {
            *s = start;
            *c = clength;
            *l = rspace;
        }
        void setValues ( unsigned char *s, unsigned char * c, unsigned int l ) {
            start = s;
            clength = c;
            rspace = l;
        }
};

class CSVReader {
    private:
        int   cNum;
        int   pCol;
        int   offset;
        int   escape;
        char  escapeChar;
        char  cols[32][256];
        void  ( *readData ) ( char **, int, void * );
        void  *data;

    public:
        CSVReader ( int cNum, void ( *readdata ) ( char **, int, void * ), void* data, char eChar );
        ~CSVReader() {}
        int parseData ( char *, int );
};

#define SEND_THREAD_DATA( socket, buffer, length ) \
{ \
unsigned int bytesW = 0; \
int temp; \
do{ \
    temp = PR_Write( (socket), (buffer)+bytesW, (length)-bytesW  ); \
    if( temp > 0 ) \
        bytesW += temp; \
    else \
    { \
        PRErrorCode error = PR_GetError(); \
        if( error == PR_WOULD_BLOCK_ERROR ) \
        { \
            PR_Sleep(1); \
        } \
        else \
        { \
            bytesW = 0; \
            break; \
        } \
    } \
} \
while( bytesW < (length) ); \
if( bytesW != (length) ) \
{ \
    fprintf(stderr,"Error: Unable to send data \n"); \
    (length) = -1; \
} else { \
    (length) = 0; \
} \
}

char  hexCharToHexBin ( char c );
void  httpencode ( char *to, char *from, int length );
void  httpdecode ( char *to, char *from );
int   httpdecodepartial ( char *to, char *from, int len );
int   convertHttpDateToNsprDate ( char *date, struct tm * );
const char* convertIndexToWeekDay ( int wdayIndex );
const char* convertIndexToMonth  ( int monthIndex );
void  convertMd5ToHexMd5  ( char *md5, char *hexmd5 );
void  convertRandomsToHex ( char *randoms, char *hex );
int   addDate ( char *hdr, const char *field, struct tm &temp );

unsigned int createChunk   ( unsigned char *buffer, unsigned int &buflen, unsigned char **start,  unsigned char **clength );
void  appendChunkData      ( unsigned char **start, unsigned int &length, unsigned int   &rspace, unsigned char  *databuf, unsigned int    dlength, unsigned char  **incompete );
void  appendChunkString    ( unsigned char **start, unsigned int &length, unsigned int   &rspace, unsigned char  *databuf, unsigned char **incompete );
void  appendChunkChar      ( unsigned char **start, unsigned int &length, unsigned int   &rspace, unsigned char   data );
void  truncateChunk        ( unsigned char *clen,   unsigned int &len );
void  lastChunk            ( unsigned char *buf,    unsigned int &len );

void  readConfigFile ( const char *fileName, const char delim, void ( *callback ) ( char *, char* ) );
int   catstring        ( char *result, char *data, int &rn );
int   mergestring      ( char *result, char *buf, char *data, int &, int & );
int   mergeremaining   ( char *result, char *buf );

#ifdef OS_WIN
char* strtok_r ( char *buf, char *delim, char **restart );
void gmtime_r ( const time_t *date,  struct tm *temp );
#endif

int rand_reentrant ( unsigned int *ctx );

#endif
