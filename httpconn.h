#ifndef HTTP_CONNECTION_H
#define HTTP_CONNECTION_H

#include <stdio.h>
#include <apptypes.h>
#include <tools.h>
#include <httpcodes.h>
#include <ctime>
//#include <prlong.h>
//#include <ifs.h>
#include <session.h>
#include <sqlite3.h>

#include <prwrapper.h>

using namespace std;

//India's offset from GMT = +5:30 hrs 19800 is in secs
enum HTTP_METHOD {
    HTTP_GET = 0,
    HTTP_POST,
    HTTP_HEAD,
    HTTP_UNKNOWN
};

enum HTTP_TAG_DREF {
    HTTP_METHOD = 0,
    HTTP_URL,
    HTTP_PROTOCOL,
    HTTP_COOKIE,
    HTTP_REFERER,
    HTTP_HOST,
    HTTP_UA,
    HTTP_ACCEPT,
    HTTP_ACCEPTENC,
    HTTP_ACCEPTCHAR,
    HTTP_ACCEPTLANG,
    HTTP_ENCODING,
    HTTP_CONTENTTYPE,
    HTTP_CONN,
    HTTP_CONTENTLEN,
    HTTP_DATE,
    HTTP_TRANSFERENC,
    HTTP_IFMODSINCE,
    HTTP_KEEPALIVE,
    MAX_HTTP_TDREF
};

enum HTTP_TAG_CONN {
    CONN_KEEPALIVE = 0,
    CONN_CLOSE
};

enum HTTP_TAG_CC {
    CC_NONE       = 0x00,
    CC_PUBLIC     = 0x01,
    CC_PRIVATE    = 0x02,
    CC_NOCACHE    = 0x04,
    CC_NOSTORE    = 0x08,
    CC_NOTRANS    = 0x10,
    CC_MUSTREVAL  = 0x20,
    CC_PROXREVAL  = 0x40,
    CC_MAX
};

enum HTTP_TAG_CENC {
    CENC_PLAIN = 0,
    CENC_GZIP,
    CENC_DEFLATE,
    CENC_MAX
};

enum AUTHLVL {
    AUTH_CUSTOMER  = 0,
    AUTH_POWERUSER = 1,
    AUTH_ADMIN     = 2
};

class HttpReq {
    private:
        char encodedUrl[1024]; // if every character is expressed as % values 256 * 3 = 768
        char decodedUrl[256];
        char authUrl[256];
        char fileType[64];
        char cookie[256];
        char referer[256];
        char host[128];
        char userAgent[128];
        char accept[128];
        char acceptEnc[64];
        char acceptChar[64];
        char acceptLang[64];
        char encoding[64];
        char contentType[128];
        char boundary[64];
        char transEnc[32];
        char dateStr[48];
        char ifModStr[48];
        char connection[32];
        char contentLen[32];
        char keepAlive[64];

        char  *query;
        int    method;
        int    multipart;
        int    version; // http 1.0 = 0 http1.1 = 1
        int    conn;

        int  i;
        int  j;
        int  postNum;
        char postFileName[256];
        bool hdrPartial;
        bool hdrReadComplete;
        bool isVal;
        bool isEnd;
        bool isTagEnd;
        bool isHdrEnd;
        char tag [128];
        char *data;

        int   processHttpFirstLine();
        void  processHttpProto ( char* );
        void  skipHdrTag ( int &, int d = 0 );
        void  processUrlQuery();
        void  processHttpMethod ( char * );
        void  processHttpHeader();
        void  processContentType();
        char* getTagBuffer ( char * );
        int   getHttpConnType ( char * );
        struct tm date;
        struct tm ifMod;

    public:

        HttpReq();
        ~HttpReq();

        int         len;
        size_t      cLen;
        size_t      hLen;
        int          postfd;
        PRFileDesc  *post;
        unsigned char buf[MAXBUF];
        unsigned char *dataPtr;
        char* getCookie()  { return cookie; }
        char* getReqFile();
        char* getReqFileType();

        char* getReqFileAuth ( int auth );
        int   getMethod () { return method; }
        bool  isMultipart () { return ( bool ) multipart; }
        char* getBoundary () { return boundary; }
        void  readHttpHeader();
        int   processHttpPostData ( int, int );
        void  convertGetDataToMap ( MapStrStr *paramMap ); //converts get parameters to a Map
        void  convertGetDataToVector ( Vector *param );
        int   convertPostDataToMap ( MapStrStr *paramMap, const char *stopAt ); //converts post parameters to a Map
        int   convertPostDataToVector ( Vector *param, const char *stopAt );
};



class HttpResp {
    private:
        float     protocol;
        int       status;
        char      server[32];
        int       contentLen;
        char      contentEnc[64];
        char      contentType[64];
        unsigned short acceptRanges;
        unsigned short connection;
        unsigned short cCtrl;
        char      location[256];
        time_t    date;
        time_t    lastModified;
        time_t    expires;
        short int addon;
        void      *data;

        int addDate ( char *hdr, const char *field, struct tm &temp );

    public:

        HttpResp();
        ~HttpResp();

        void setAddOn ( short int a )
        { addon = 1; }
        void setContentType ( const char *contentType );
        void setContentEnc  ( const char *contentEnc );
        void setCacheControl ( unsigned short );
        void setStatus      ( int status );
        void setContentLen  ( int contentLen );
        void setLocation    ( const char * );

        void setLastModifiedDate ( time_t );
        void setExpiry ( time_t );
        void setData  ( void *d )
        {   data = d; }

        void * getData()
        { return data; }
        int  getStatus();
        int  getContentLen();

        void resetHttpHeader();
        int  getHeader ( char *hdr );
        int  finishHdr ( char *hdr );
        int  finishChunked ( char *hdr );
};




class Connection {
    public:
        unsigned char buf[SMALLBUF];
        unsigned int  sent;
        unsigned int  len;
        unsigned int  offset;
        unsigned int  ip;
        unsigned int  authLvl;
        unsigned int  cmd;
        int32_t        filefd;
        PRFileDesc    *file;
        PRFileInfo64  fInfo;
        int32_t        socketfd;
        PRFileDesc    *socket;
        sqlite3       *db;
        //FileIndexData *fid;
        void *fid;
        HttpReq       req;
        HttpResp      resp;
        HttpSession   *sess;
        void          *udata;

        Connection() {
            len = 0;
            sent = 0;
            offset = 0;
            ip     = 0;
            authLvl = AUTH_CUSTOMER;
            cmd    = 0;
			filefd   = -1;
            file = &filefd;
			socketfd = -1;
            socket = &socketfd;
            fid = NULL;
            udata = NULL;
            sess  = NULL;
            db    = NULL;
        }

        ~Connection() {
            if ( socket )
            { close ( *socket ); }

            if ( file )
            { close ( *file ); }

            len    = 0;
            sent   = 0;
            offset = 0;
            ip     = 0;
            authLvl = AUTH_CUSTOMER;
            cmd    = 0;
            filefd = -1;
            socketfd = -1;
            socket = NULL;
            file   = NULL;
            fid    = NULL;
            udata  = NULL;
            sess   = NULL;
            db     = NULL;
        }

        void setAuthLvl() {
            fprintf ( stderr, "DBUG: ip = 0x%x and mask = 0x%x \n", ip, ( ip & 0xffff0000 ) );

            if ( ip == 0x7f000001 ) {
                authLvl = AUTH_ADMIN;
                fprintf ( stderr, "INFO: Setting user to Admin LVl \n" );
            } else if ( ( ip & 0xffff0000 ) == 0xc0a80000 ) {
                authLvl = AUTH_POWERUSER;
                fprintf ( stderr, "INFO: Setting user to Power User LVl \n" );
            } else {
                authLvl = AUTH_CUSTOMER;
                fprintf ( stderr, "INFO: Setting user to Customer Lvl \n" );
            }

        }

};

unsigned int sendConnRespHdr    ( Connection *conn, int status = HTTP_RESP_OK );
unsigned int sendConnectionData ( PRFileDesc *socket, unsigned char *buffer, unsigned int length );
unsigned int sendConnectionData ( Connection *conn, unsigned char *buffer, unsigned int length );
void         sendRemainderData  ( Connection *conn );

#endif
