//Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3
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
#include <defines.h>

#ifdef USE_SSL
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#include <log.h>

using namespace std;

//India's offset from GMT = +5:30 hrs 19800 is in secs
enum HTTP_METHOD {
    HTTP_GET = 0,
    HTTP_POST,
    HTTP_HEAD,
    HTTP_PUT,
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

/* same values should be present in ulogin table in INFO_STORE */
enum AUTHLVL {
    AUTH_PUBLIC     = 1, /*lower power */
    AUTH_ADMIN      = 2,
    AUTH_ROOTUSER   = 3,
    AUTH_SUPERUSER  = 4 /*higher power*/
};

class Connection;

class HttpReq {
    private:
        char encodedUrl[4096]; // if every character is expressed as % values 256 * 3 = 768
        char decodedUrl[1024];
        //char authUrl[2048]; //there maybe a prefix, so the size is increased
        string authUrl;
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

        size_t  i;
        size_t  j;

        int  postNum;
        char postFileName[1024];
        bool hdrPartial;
        bool hdrReadComplete;
        bool hdrInvalid;
        bool isVal;
        bool isEnd;
        bool isTagEnd;
        bool isHdrEnd;
        char tag [128];
        char *data;

        int   processHttpFirstLine();
        void  processHttpProto ( char* );
        void  skipHdrTag ( size_t &, size_t d = 0 );
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

        size_t       len;
        size_t      cLen;
        size_t      rLen;
        size_t      hLen;

        int          postfd;
        PRFileDesc  *post;
        unsigned char buf[MAXBUF];
        unsigned char *dataPtr;
        char* getCookie()  { return cookie; }
        char* getReqFile();
        char* getReqFileType();

        char* getReqFileAuth ( );
        char* getReqFileAuth ( int auth );
        int   getMethod () { return method; }
        char* getPostTempFileName ( ) {
            return postFileName;
        }
        void  removePostTempFile() {
            debuglog (  "DBUG: Removing post temp file: %s \n", postFileName );

            if ( post && postfd >= 0 ) {
                PR_Close ( post );
                post = 0;
                postfd = -1;
            } else {
                post = 0;
                postfd = -1;
            }

            PR_Unlink ( postFileName );
            postFileName[0] = 0;
        }
        bool  isMultipart () { return ( bool ) multipart; }
        char* getBoundary () { return boundary; }
        void  readHttpHeader();
        bool  isHdrInvalid() { return hdrInvalid; }
        int   processHttpPostData ( size_t, size_t );
        int   processHttpPostData ( Connection *conn ); //to be used from plugins only
        void  convertGetDataToMap ( MapStrStr *paramMap ); //converts get parameters to a Map
        void  convertGetDataToVector ( Vector *param );
        int   convertPostDataToMap ( MapStrStr *paramMap, const char *stopAt ); //converts post parameters to a Map
        int   convertPostDataToVector ( Vector *param, const char *stopAt );
};



class HttpResp {
    private:
        float     protocol;
        int       status;
        char      server[64];
        int       contentLen;
        char      contentEnc[64];
        char      contentType[64];
        unsigned short acceptRanges;
        unsigned short connection;
        unsigned short cCtrl;
        char      location[1024];
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
		static long long int indexCount;
		long long int            index;
		bool           delobj;
        unsigned char  buf[SMALLBUF];
        unsigned int   sent;
        unsigned int   len;
        unsigned int   offset;
#ifdef USE_SSL
        //0 - nossl, 1 ssv1, 2 sslv2, 3 sslv3, 100 tls 1.0, 101 tls 1.1, 102 tls 1.2, ... etc
        SSL            *ssl;
        bool           ssl_accept;
        bool           is_ssl;
#endif
        unsigned int   ip;
        unsigned short ipv6[8];

        unsigned int  authLvl;
        unsigned int  cmd;
        int32_t        filefd;
        PRFileDesc    *file;
        PRFileInfo64  fInfo;
        int32_t        socketfd;
        PRFileDesc    *socket;
        sqlite3       *db; //link to per thread sqlite3 db
        //FileIndexData *fid;
        void *fid;
        HttpReq       req;
        HttpResp      resp;
        HttpSession   *sess;
        void          *udata;

        Connection() {
			index   = indexCount++;
			delobj  = false;
            debuglog( " Connection Object created with index-%ld \n", index );
            len     = 0;
            sent    = 0;
            offset  = 0;
#ifdef USE_SSL
            is_ssl  = false;
            ssl_accept = false;
            ssl     = 0;
#endif
            ip      = 0;
            ipv6[0] = 0;
            ipv6[1] = 0;
            ipv6[2] = 0;
            ipv6[3] = 0;
            ipv6[4] = 0;
            ipv6[5] = 0;
            ipv6[6] = 0;
            ipv6[7] = 0;
            authLvl = AUTH_PUBLIC;
            cmd    = 0;
            filefd   = -1;
            file = &filefd;
            socketfd = -1;
            socket = &socketfd;
            fid = 0;
            udata = 0;
            sess  = 0;
            db    = 0;
        }

        ~Connection() {
            if ( socket )
            { PR_Close ( socket ); }

            if ( file )
            { PR_Close ( file ); }

			if( index < 0 )
				debuglog( " Connection Object already deleted with index%ld \n", index);
			else 
            	debuglog( " Connection Object deleted with index-%ld \n", index );

			index   = -index;
            len     = 0;
            sent    = 0;
            offset  = 0;
#ifdef USE_SSL
            is_ssl  = false;
            ssl_accept = false;
            ssl     = 0;
#endif
            ip      = 0;
            ipv6[0] = 0;
            ipv6[1] = 0;
            ipv6[2] = 0;
            ipv6[3] = 0;
            ipv6[4] = 0;
            ipv6[5] = 0;
            ipv6[6] = 0;
            ipv6[7] = 0;
            authLvl = AUTH_PUBLIC;
            cmd    = 0;
            filefd = -1;
            socketfd = -1;
            socket = 0;
            file   = 0;
            fid    = 0;
            udata  = 0;
            //donot free sess, as there could other connection from the same system trying to access a different resource.
            //instead just reset the pointer, the sess is present in an internal stl::map, its not considered memory leak.
            //if( sess ){
            //    delete sess;
            //}
            sess   = 0;
            db     = 0;
        }

        void setAuthLvl() {
            if ( sess ) {
                authLvl = atoi ( sess->getVariable ( "auth" ) );

                if ( authLvl < AUTH_PUBLIC || authLvl > AUTH_SUPERUSER )
                { authLvl = AUTH_PUBLIC; }
            } else {
                authLvl = AUTH_PUBLIC;
            }
        }
};

unsigned int sendConnRespHdr    ( Connection *conn, int status = HTTP_RESP_OK );
unsigned int sendConnectionDataToSock ( PRFileDesc *socket, unsigned char *buffer, unsigned int length );
unsigned int sendConnectionData ( Connection *conn, unsigned char *buffer, unsigned int length );
void         sendRemainderData  ( Connection *conn );

#endif
