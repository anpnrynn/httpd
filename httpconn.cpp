//Copyright Anoop Kumar Narayanan <anoop.kumar.narayanan@gmail.com> , LICENSE - GPLv2 / GPLv3
#include <httpconn.h>
#include <tools.h>


#ifndef LINUX_BUILD
#include <cstring>
//#define strcasecmp _stricmp
#endif // LINUX_BUILD

#include <thread>
#include <chrono>
using namespace std;


//int gmtOffset =  GMT_INDIA;
//const char *srvName = "AppServer v0.0.6";
#include "version.h"
const char *srvName = VERSION;

HttpReq::HttpReq() {
    len = 0;
    encodedUrl[0] = 0;
    decodedUrl[0] = 0;
    //authUrl[0] = 0;
    authUrl = "";
    fileType[0] = 0;
    cookie[0] = 0;
    referer[0] = 0;
    host[0] = 0;
    userAgent[0] = 0;
    accept[0] =  0;
    acceptEnc[0] = 0;
    acceptChar[0] = 0;
    acceptLang[0] = 0;
    encoding[0] = 0;
    contentType[0] = 0;
    boundary[0] = 0;
    transEnc[0] = 0;
    dateStr[0] = 0;
    ifModStr[0] = 0;
    connection[0] = 0;
    contentLen[0] = 0;
	termId[0] = 0;
    method = -1;
    version = -1;
    string rawHdr = "";

    i = 0;
    j = 0;
    postFileName[0] = 0;
    hdrPartial = false;
    hdrReadComplete = false;
    hdrInvalid = false;
    isVal =  false;
    isEnd =  false;
    isTagEnd = false;
    isHdrEnd = false;
    tag[0]   = 0;
    data =  NULL;
    conn = 1;
    cLen = 0;
    rLen = 0;
    hLen = 0;
    query = NULL;
    dataPtr  = NULL;
    post     = NULL;
}

HttpReq::~HttpReq() {
    if ( post ) {
        removePostTempFile();
    }
}

void HttpReq::processHttpMethod ( char* mthd ) {
    if ( strcasecmp ( "GET", mthd ) == 0 )
    { method = HTTP_GET; }
    else if ( strcasecmp ( "POST", mthd ) == 0 )
    { method = HTTP_POST; }
    else if ( strcasecmp ( "HEAD", mthd ) == 0 )
    { method = HTTP_HEAD; }
    else
    { method = HTTP_METHOD_MAX; hdrInvalid = true; }

}

void HttpReq::processHttpProto ( char *prot ) {
    if ( strcasecmp ( "HTTP/1.0", prot ) == 0 )
    { version = 0; }
    else if ( strcasecmp ( "HTTP/1.1", prot ) == 0 )
    { version = 1; }
    else { version = -1; hdrInvalid = true; }
}

void HttpReq::skipHdrTag ( size_t &start, size_t d ) {
    size_t k = start;

    while ( k < len - 1 && ( buf[k] != '\r' && buf[k + 1] != '\n' ) && buf[k] != '\n' ) {
        k++;
    }

    if ( buf[k] == '\r' )
    { start = k + 2 - d; }
    else if ( buf[k] == '\n' )
    { start = k + 1 - d; }
    else
    { start = k - d; }
}

bool isHttpMethod ( char *mthd ) {
    if ( strcmp ( mthd, "GET" ) != 0 &&
            strcmp ( mthd, "POST" ) != 0 &&
            strcmp ( mthd, "HEAD" ) != 0 &&
            strcmp ( mthd, "PUT" )  != 0 &&
            strcmp ( mthd, "PATCH" )  != 0 &&
            strcmp ( mthd, "DELETE" )  != 0 ) {
        return false;
    } else {
        return true;
    }
}

int HttpReq::processHttpFirstLine() {
    char mthd[16];
    char prot[16];
    int i = 0;
    int j = 0;
    int k = 0;
    char *hdrReq[3] = { mthd, encodedUrl, prot };

    while ( buf[i] < MAXBUF ) {
        if ( buf[i] == ' ' ) {
            hdrReq[j][k] = 0;
            j++;
            k = 0;

            if ( j == 1 ) {
                if ( !isHttpMethod ( mthd ) ) {
                    hdrInvalid = true;
                    return 10;
                }
            } else if ( j > 3 ) {
                hdrInvalid = true;
                return 11;
            }

            if ( buf[i + 1] == '/' ) {
                i++;
            }
        } else if ( buf[i] == '\r' || buf[i] == '\n' ) {
            hdrReq[j][k] = 0;
            j++;
            k = 0;
            break;
        } else {
            if ( ( j == 0 || j == 2 ) && k >= 15 ) {
                hdrInvalid = true;
                return 12;
            } else if ( j == 1 && k >= 4095 ) {
                hdrInvalid = true;
                return 13;
            }

            hdrReq[j][k++] = buf[i];
        }

        i++;
    }

    if ( j == 3 ) {
        if ( encodedUrl[0]  == 0 ) {
            strcpy ( encodedUrl, "index.html" );
        }

        debuglog (  " Method=%s Version=%s EncodedUrl=%s \n", mthd, prot, encodedUrl );
        hdrInvalid = false;
    }  else {
        debuglog (  "ERRR: Header invalid <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n" );
        hdrInvalid = true;
        return 10;
    }

    processHttpMethod ( mthd );
    processHttpProto ( prot );
    httpdecode       ( decodedUrl, encodedUrl );

    return j;
}

int HttpReq::getHttpConnType ( char *data ) {
    if ( !data )
    { return CONN_CLOSE; }

    if ( strcasecmp ( "Keep-Alive", data ) == 0 )
    { return CONN_KEEPALIVE; }
    else if ( strcasecmp ( "Close", data ) ==  0 )
    { return CONN_CLOSE; }
    else
    { return CONN_CLOSE; }
}

char* HttpReq::getTagBuffer ( char *tag ) {
    if ( !tag )
    { return NULL; }

    if ( strcasecmp ( "Host", tag ) == 0 )
    { return host; }
    else if ( strcasecmp ( "Accept", tag ) == 0 )
    { return accept; }
    else if ( strcasecmp ( "Accept-Encoding", tag ) == 0 )
    { return acceptEnc; }
    else if ( strcasecmp ( "Accept-Charset", tag ) == 0 )
    { return acceptChar; }
    else if ( strcasecmp ( "Accept-Language", tag ) == 0 )
    { return acceptLang; }
    else if ( strcasecmp ( "User-Agent", tag ) == 0 )
    { return userAgent; }
    else if ( strcasecmp ( "Referer", tag ) == 0 )
    { return referer; }
    else if ( strcasecmp ( "Cookie", tag ) == 0 )
    { return cookie; }
    else if ( strcasecmp ( "Encoding", tag ) == 0 )
    { return encoding; }
    else if ( strcasecmp ( "Content-Type", tag ) == 0 )
    { return contentType; }
    else if ( strcasecmp ( "Date", tag ) == 0 )
    { return dateStr; }
    else if ( strcasecmp ( "Transfer-Encoding", tag ) == 0 )
    { return transEnc; }
    else if ( strcasecmp ( "If-Modified-Since", tag ) == 0 )
    { return ifModStr; }
    else if ( strcasecmp ( "Content-Length", tag ) == 0 )
    { return contentLen; }
    else if ( strcasecmp ( "Connection", tag ) == 0 )
    { return connection; }
    else if ( strcasecmp ( "Keep-Alive", tag ) == 0 )
    { return keepAlive; }
	else if ( strcasecmp ( "Term-Id", tag ) == 0 ){
	  return termId;	
    }else
    { return NULL; }
}

void HttpReq::readHttpHeader() {

    if ( hdrInvalid ) {
        return;
    }

    if ( !hdrPartial ) {
        hdrPartial = true;

        if ( processHttpFirstLine() != 3 ) {
            debuglog (  "ERRR: Invalid Http Header, dropping connection \n" );
            hdrInvalid = true;
            hLen = ( size_t ) 0;
            return;
        }

        if ( hdrInvalid ) {
            return;
        }

        skipHdrTag ( i );
    }

    while ( i < len  && !hdrReadComplete ) {
        switch ( buf[i] ) {
            case ' ':
                if ( isVal && j > 0 )
                { data[j++] = buf[i]; }

                break;

            case '\r':
                if ( isEnd && buf[i - 1] == '\n' )
                { isHdrEnd = true; }

                if ( data ) {
                    data[j] = 0;
                    j = 0;
                    data = NULL;
                }

                isTagEnd  = false;
                isVal     = false;
                isEnd = true;
                break;

            case '\n':
                if ( isEnd && buf[i - 1] == '\n' )
                { isHdrEnd = true; }

                if ( !isEnd ) {
                    isTagEnd = false;
                    isVal    = false;
                    isEnd   = true;

                    if ( data ) {
                        data[j] = 0;
                        j = 0;
                        data = NULL;
                    }
                }

                if ( isHdrEnd ) {
                    hdrReadComplete = true;
                }

                break;

            case ':':
                if ( !isTagEnd ) {
                    tag[j]   = 0;
                    isTagEnd = true;
                    isVal    = true;
                    j = 0;
                    data = NULL;
                    data = getTagBuffer ( tag );

                    if ( !data ) {
                        skipHdrTag ( i, 1 );
                        //printf("Skipping Header: %s \n", tag);
                        isTagEnd = false;
                        isEnd = true;
                        isVal = false;
                    }
                } else {
                    data[j++] = buf[i];
                }

                break;

            default:
                if ( isTagEnd ) {
                    data[j++] = buf[i];
                    isVal     = true;
                } else {
                    tag[j++]  = buf[i];

                    if ( isEnd ) {
                        isEnd = false;
                        isHdrEnd = false;
                    }
                }

                break;
        }

        i++;
    }

    if ( hdrReadComplete ) {
        buf[i - 1] = 0;
        rawHdr = ( const char* ) buf;
        hLen   = i;
        debuglog (  "XTRA: ____________________________________\n" );
        debuglog (  "XTRA: Request Header:\n%s\n\n", ( char * ) buf );
        processHttpHeader();
        processContentType();

        if ( hLen < MAXBUF )
        { dataPtr = &buf[hLen]; }
    }
}


void HttpReq::processContentType() {
    if ( contentType[0] != 0 ) {
        char *boundaryPos = strstr ( contentType, "boundary=" );

        if ( boundaryPos ) {
            strcpy ( boundary, boundaryPos + 9 );
            debuglog (  "DBUG: Multipart Content Type, Boundary String Retrieved = '%s' '%s' \n", boundaryPos + 9, boundary );
            multipart = 1;
        } else {
            debuglog (  "DBUG: Boundary String not detected, not multipart \n" );
            multipart = 0;
        }
    }
}

void HttpReq::processHttpHeader() {
    if ( connection[0] != 0 ) {
        conn = getHttpConnType ( connection );
    }

    if ( cookie[0] != 0 ) {
        httpdecode ( cookie, cookie );
    } else {
        //Create a session object
    }

    if ( contentLen[0] != 0 ) {
        cLen = atoll ( contentLen );
    }

    if ( dateStr[0] != 0 ) {
        convertHttpDateToNsprDate ( dateStr, &date );
    }

    if ( ifModStr[0] != 0 ) {
        convertHttpDateToNsprDate ( ifModStr, &ifMod );
    }

    query = strchr ( decodedUrl, '?' );

    if ( !query ) {
        //TODO:
        //query = decodedUrl;
        query = 0;
    } else {
        *query = 0;
        query++;
    }


    if ( ( strcmp ( decodedUrl, "HTTP/1.1" ) != 0 ) && ( strcmp ( decodedUrl, "HTTP/1.0" ) != 0 ) ) {
        char *ft = 0;
        ft = strrchr ( decodedUrl, ( int ) '.' );

        if ( ft ) {
            int i = 0;

            while ( *ft != '\0' ) {
                fileType[i] = *ft;
                i++;
                ft++;
            }

            fileType[i] = 0;
            debuglog (  "DBUG: Requested file type = '%s' \n", fileType );
        }
    } else {
        strcpy ( decodedUrl, "index.html" );
        strcpy ( fileType, ".html" );
    }

    debuglog (  "DBUG: Request file = '%s' Query = '%s' \n", decodedUrl, query ? query : "" );
}

char* HttpReq::getReqFile() {
    //debuglog (  "DBUG: Query = '%s' \n", query?query:"");
    return decodedUrl;
}


char *HttpReq::getReqFileType() {
    return fileType;
}

char *HttpReq::getReqFileAuth ( ) {
    if ( authUrl == "" ) {
        //sprintf( authUrl, "%s%s", PAGE_STORE, decodedUrl );
        //strcat( authUrl, PAGE_STORE );
        //strcat( authUrl, decodedUrl );
        authUrl += PAGE_STORE;
        authUrl += decodedUrl;
    }

    return const_cast<char*> ( authUrl.c_str() );
}

char *HttpReq::getReqFileAuth ( int auth ) {
    if ( authUrl == "" ) {

#ifdef LINUX_BUILD
        //strcpy ( authUrl, PAGE_STORE );
        authUrl += PAGE_STORE;
#else
        //strcpy ( authUrl, PAGE_STORE );
        authUrl += PAGE_STORE;
#endif

        if ( strcasecmp ( decodedUrl, "login.html" ) == 0 ||
                strcasecmp ( decodedUrl, "public.html" ) == 0 ||
                strcasecmp ( decodedUrl, "index.html" ) == 0 ) {
            //strcat ( authUrl, decodedUrl );
            authUrl += decodedUrl;
            return const_cast<char*> ( authUrl.c_str() );
        }

        switch ( auth ) {
            case AUTH_SUPERUSER:
                //strcat ( authUrl, "super_" );
                authUrl += "super_";
                break;

            case AUTH_ROOTUSER:
                //strcat ( authUrl, "root_" );
                authUrl += "root_";
                break;

            case AUTH_ADMIN:
                //strcat ( authUrl, "admin_" );
                authUrl += "admin_";
                break;

            case AUTH_PUBLIC:
            default:
                //strcat ( authUrl, "all_" );
                authUrl += "all_";
                break;
        };

        if ( ( strcmp ( decodedUrl, "HTTP/1.1" ) == 0 ) || ( strcmp ( decodedUrl, "HTTP/1.0" ) == 0 ) ) {
            //strcat ( authUrl, "index.html" );
            authUrl += "index.html";
        } else {
            //strcat ( authUrl, decodedUrl );
            authUrl += decodedUrl;
        }

        debuglog (  "DBUG: Auth Request file = '%s' \n", authUrl.c_str() );
        return const_cast<char*> ( authUrl.c_str() );
    } else {
        debuglog (  "DBUG: Auth Request file = '%s' \n", authUrl.c_str() );
        return const_cast<char*> ( authUrl.c_str() );
    }
}

int postCount = 0;

int HttpReq::processHttpPostData ( Connection *conn ) {
    if ( conn && post ) {
        while ( rLen < cLen ) {
            size_t bytesR  = PR_Recvfd ( * ( conn->socket ), buf, MAXBUF, 0, 1 );
            size_t bytesW  = PR_Write ( post, buf, bytesR );
            rLen += bytesR;

            if ( bytesW < bytesR ) {
                debuglog (  "ERRR: Bytes dropped , read = %ld, written = %ld \n", bytesR, bytesW );
            }
        }

        return rLen;
    }

    return -1;
}

int HttpReq::processHttpPostData ( size_t hPkt, size_t dLen ) {
    if ( ( hPkt > 0 ) && !post ) {
        //char fName[64];
        postNum = postCount;
#ifdef LINUX_BUILD
        //sprintf( fName,"%s/post_%d.tmp","/tmp",postCount );
        sprintf ( postFileName, "%s/post_%d.tmp", "/tmp", postCount );
#else
        //sprintf( fName,"%s\\post_%d.tmp","tmp",postCount );
        sprintf ( postFileName, "%s\\post_%d.tmp", "tmp", postCount );
#endif
        postCount++;
        post = &postfd;
        //post = PR_Open( fName, PR_CREATE_FILE|PR_TRUNCATE|PR_RDWR, 660 );
        *post = PR_Open ( postFileName, PR_CREATE_FILE | PR_TRUNCATE | PR_RDWR, PR_IRWXO | PR_IRWXG | PR_IRWXU );

        if ( !post ) {
            debuglog (  "ERRR: Unable to open post data file '%s'\n", postFileName );
            return -1;
        } else
        { debuglog (  "INFO: Opened file : %s \n", postFileName ); }
    }

    if ( post ) {
        size_t tempLen = 0;
        size_t bytesW  = 0;

        //int bufLen = hPkt ? (len-hLen): dLen;
        while ( tempLen < dLen ) {
            bytesW = PR_Write ( post, &buf[hLen], dLen - tempLen );

            if ( bytesW <= 0 )
            { break; }
            else
            { tempLen += bytesW; }
        }

        //printf("INFO: Wrote %d bytes\n",bytesW);
        rLen += tempLen;
        return tempLen;
    }

    return -1;
}


void HttpReq::convertGetDataToMap ( MapStrStr *param ) {
    char qname[2048];
    char qvalue[2048];
    char *data = qname;
    int n = strlen ( query );
    int i = 0;
    //printf("Query String=%s\n",query);
    int j = 0;

    if ( query[i] != '\0' && i < n ) {
        for ( i = 0; i < n ; i++ ) {
            if ( query[i] == '=' ) {
                data[j] = 0;
                data = qvalue;
                j = 0;
            } else if ( query[i] == '&' ) {
                data[j] = 0;
                //printf("Name=%s Value=%s \n", qname, qvalue);
                param->insert ( pair<string, string> ( qname, qvalue ) );
                data = qname;
                j = 0;
            } else {
                data[j++] = query[i];
            }
        }
    }

    if ( j > 0 && data == qvalue ) {
        data[j] = 0;
        param->insert ( pair<string, string> ( qname, qvalue ) );
    }
}

void HttpReq::convertGetDataToVector ( Vector *param ) {
    char qname[2048];
    char qvalue[2048];
    char *data = qname;
    int n = strlen ( query );
    int i = 0;
    //printf("Query String=%s\n",query);
    int j = 0;

    if ( query[i] != '\0' && i < n ) {
        for ( i = 0; i < n ; i++ ) {
            if ( query[i] == '=' ) {
                data[j] = 0;
                data = qvalue;
                j = 0;
            } else if ( query[i] == '&' ) {
                data[j] = 0;
                //printf("Name=%s Value=%s \n", qname, qvalue);
                param->push_back ( qvalue );
                data = qname;
                j = 0;
            } else {
                data[j++] = query[i];
            }
        }
    }

    if ( j > 0 && data == qvalue ) {
        data[j] = 0;
        param->push_back ( qvalue );
    }
}

int HttpReq::convertPostDataToMap ( MapStrStr *param, const char *stopAt ) {
    unsigned int k = 0, l = 0;

    if ( contentLen[0] == 0 )
    { return 0; }

    //char name [256];
    //char value[256];
    char *name   = new char[MAXBUF];

    if ( !name )
    { return 1; }

    char *value  = new char[MAXBUF];

    if ( !value )
    { delete []name; return 1; }

    char *varPtr = name;
    unsigned int  j = 0, dataLen = 0;

    //TODO
    PR_Seek64 ( post, 0, PR_SEEK_SET );

    while ( ( dataLen = PR_Read ( post, &buf[l], MAXBUF - l ) ) > 0 ) {
        dataLen += l;
        l = 0;

        for ( k = 0; k < dataLen; k++ ) {
            if ( buf[k] == '&' ) {
                varPtr[j++] = 0;
                j = 0;
                param->insert ( pair<string, string> ( name, value ) );
                name[0]  = 0;
                value[0] = 0;
                varPtr = name;
            } else if ( buf[k] == '=' ) {
                varPtr[j++] = 0;

                if ( stopAt ) {
                    if ( strcmp ( name, stopAt ) ) {
                        delete []name;
                        delete []value;
                        return 1;
                    }
                }

                j = 0;
                varPtr = value;
            } else if ( buf[k] == '+' ) {
                varPtr[j++] = ' ';
            } else if ( buf[k] == '%' ) {
                if ( k <= ( dataLen - 3 ) ) {
                    if ( isxdigit ( buf[k + 1] ) && isxdigit ( buf[k + 2] ) ) {
                        varPtr[j++] = hexCharToHexBin ( buf[k + 1] ) * 16 + hexCharToHexBin ( buf[k + 2] );
                        k += 2;
                    }
                } else {
                    buf[0] = buf[k];
                    buf[1] = buf[k + 1];
                    l = 2;
                    break;
                }
            } else {
                varPtr[j++] = buf[k];
            }
        }
    }

    varPtr[j++] = 0;
    j = 0;

    if ( name[0] != 0 ) {
        param->insert ( pair<string, string> ( name, value ) );
    }

    if ( name )
    { delete []name; }

    if ( value )
    { delete []value; }


    return 0;
}

int HttpReq::convertPostDataToVector ( Vector *param, const char *stopAt ) {
    unsigned int k = 0, l = 0;

    if ( contentLen[0] == 0 )
    { return 0; }

    char *name   = new char[MAXBUF];

    if ( !name )
    { return 1; }

    char *value  = new char[MAXBUF];

    if ( !value )
    { delete []name; return 1; }

    char *varPtr = name;
    unsigned int  j = 0, dataLen = 0;

    //TODO:
    PR_Seek64 ( post, 0, PR_SEEK_SET );

    while ( ( dataLen = PR_Read ( post, &buf[l], MAXBUF - l ) ) > 0 ) {
        dataLen += l;
        l = 0;

        for ( k = 0; k < dataLen; k++ ) {

            if ( buf[k] == '&' ) {
                varPtr[j++] = 0;
                j = 0;
                param->push_back ( varPtr );
                name[0]  = 0;
                value[0] = 0;
                varPtr = name;
            } else if ( buf[k] == '=' ) {
                varPtr[j++] = 0;

                if ( stopAt ) {
                    if ( strcmp ( name, stopAt ) ) {
                        //free ( name );
                        //free ( value );
                        delete []name;
                        delete []value;
                        return 1;
                    }
                }

                j = 0;
                varPtr = value;
            } else if ( buf[k] == '+' ) {
                varPtr[j++] = ' ';
            } else if ( buf[k] == '%' ) {
                if ( k <= ( dataLen - 3 ) ) {
                    if ( isxdigit ( buf[k + 1] ) && isxdigit ( buf[k + 2] ) ) {
                        varPtr[j++] = hexCharToHexBin ( buf[k + 1] ) * 16 + hexCharToHexBin ( buf[k + 2] );
                        k += 2;
                    }
                } else {
                    buf[0] = buf[k];
                    buf[1] = buf[k + 1];
                    l = 2;
                    break;
                }
            } else {
                varPtr[j++] = buf[k];
            }
        }
    }

    if ( j > 0 && value[0] != 0 ) {
        varPtr[j++] = 0;
        param->push_back ( varPtr );
        j = 0;
    }

#if 0
    int p = 0;

    while ( p < param->size () ) {
        debuglog (  "Parameter %d: %s\n", p, ( ( *param ) [p] ).c_str() );
        p++;
    }

#endif

    if ( name )
    { delete []name; }

    if ( value )
    { delete []value; }

    //free( name );
    //free( value);
    return 0;
}

HttpResp::HttpResp() {
    protocol = 1.1;
    status   = HTTP_RESP_OK;
    strcpy ( server, srvName );
    contentLen    = 0;
    acceptRanges  = 0;
	termId[0]     = 0;
    cCtrl  = CC_NONE;
    date   = time ( NULL );
    strcpy ( contentType, "text/plain; charset=UTF-8" );
    location[0]   = 0;
    contentEnc[0] = 0;
    connection    = CONN_CLOSE;
    lastModified  = 0;
    expires       = 0;
    data          = 0;
    addon         = 0;
}

HttpResp::~HttpResp() {
}

void HttpResp::setContentType ( const char *cType ) {
    if ( cType ) {
        strcpy ( contentType, cType );
    }
}

void HttpResp::setContentEnc  ( const char *cEnc ) {
    if ( cEnc ) {
        strcpy ( contentEnc, cEnc );
    }
}

void HttpResp::setTermId ( char *tid ){
    if ( tid ) {
        strcpy ( termId, tid );
    }
}

void HttpResp::setTermId ( int tid ){
    if ( tid > 0) {
        snprintf ( termId,15, "%d", tid );
    }
}

void HttpResp::setCacheControl ( unsigned short cacheCtrl ) {
    cCtrl = cacheCtrl;
}

void HttpResp::setStatus      ( int s ) {
    status = s;
}

void HttpResp::setContentLen  ( int cLen ) {
    contentLen = cLen;
}

void HttpResp::setLocation    ( const char *loc ) {
    if ( loc ) {
        strcpy ( location, loc );
    }
}

void HttpResp::setLastModifiedDate ( time_t time ) {
    lastModified = time;
}

void HttpResp::setExpiry ( time_t time ) {
    expires = time;
}

int  HttpResp::getStatus() {
    return status;
}

int  HttpResp::getContentLen() {
    return contentLen;
}


int HttpResp::addDate ( char *hdr, const char *field, struct tm &temp ) {
    return sprintf ( hdr, "%s: %s, %s%d %s %4d %s%d:%s%d:%s%d GMT\r\n",
                     field, convertIndexToWeekDay ( temp.tm_wday ), temp.tm_mday < 10 ? "0" : "", temp.tm_mday, convertIndexToMonth ( temp.tm_mon ),
                     ( 1900 + temp.tm_year ), temp.tm_hour < 10 ? "0" : "", temp.tm_hour, temp.tm_min < 10 ? "0" : "", temp.tm_min, temp.tm_sec < 10 ? "0" : "", temp.tm_sec );
}

int HttpResp::getHeader ( char *hdr ) {
    int bytesW = -1;

    if ( !hdr )
    { return -1; }

    if ( status == HTTP_RESP_OK )
    { bytesW = sprintf ( hdr, "HTTP/%1.1f 200 OK\r\n", protocol ); }
    else
    { bytesW = sprintf ( hdr, "HTTP/%1.1f %d %s\r\n", protocol, status, getStatusInfo ( status ) ); }

    struct tm temp;

    gmtime_r ( ( const time_t * ) &date,  &temp );

    bytesW += addDate ( &hdr[bytesW], "Date", temp );

    bytesW += sprintf ( &hdr[bytesW], "Server: %s\r\n", server );

    bytesW += sprintf ( &hdr[bytesW], "Connection: %s\r\n", connection == 0 ? "Keep-Alive" : "Close" );

	if( termId[0] != 0 )
    	bytesW += sprintf ( &hdr[bytesW], "Term-Id: %s\r\n", termId);

    if ( contentLen < 0 )
    { bytesW += sprintf ( &hdr[bytesW], "Transfer-Encoding: chunked\r\n" ); }
    else
    { bytesW += sprintf ( &hdr[bytesW], "Content-Length: %d\r\n", contentLen ); }

    if ( contentType[0] != 0 ) {
        bytesW += sprintf ( &hdr[bytesW], "Content-Type: %s\r\n", contentType );
    }

    if ( cCtrl ) {
        bytesW += sprintf ( &hdr[bytesW], "Pragma: no-cache\r\nCache-Control: %s%s%s%s%s%s%s\r\n", cCtrl & CC_PUBLIC ? " public," : "",
                            cCtrl & CC_PRIVATE ? " private," : "", cCtrl & CC_NOCACHE ? " no-cache," : "", cCtrl & CC_NOSTORE ? " no-store," : "", cCtrl & CC_NOTRANS ? " no-transform" : "",
                            cCtrl & CC_MUSTREVAL ? " must-revalidate," : "", cCtrl & CC_PROXREVAL ? " proxy-revalidate," : "" );
    }

    if ( lastModified ) {
        gmtime_r ( ( const time_t * ) &lastModified,  &temp );
        bytesW += addDate ( &hdr[bytesW], "Last-Modified", temp );
    }

    if ( expires ) {
        gmtime_r ( ( const time_t * ) &expires,  &temp );
        bytesW += addDate ( &hdr[bytesW], "Expires", temp );
    }

    if ( !acceptRanges ) {
        bytesW += sprintf ( &hdr[bytesW], "Accept-Ranges: none\r\n" );
    }

    if ( contentEnc[0] != 0 ) {
        bytesW += sprintf ( &hdr[bytesW], "Content-Encoding: %s\r\n", contentEnc );
    }

    if ( location[0] != 0 ) {
        bytesW += sprintf ( &hdr[bytesW], "Location: %s\r\n", location );
    }

    //if ( addon == 0 )
    //{ bytesW += sprintf ( &hdr[bytesW], "\r\n" ); }

    return bytesW;
}

int  HttpResp::finishHdr ( char *buf ) {
    return sprintf ( buf, "\r\n" );
}

int  HttpResp::finishChunked ( char *buf ) {
    if ( contentLen < 0 ) {
        return sprintf ( buf, "0\r\n\r\n" );
    }

    return 0;
}

void HttpResp::resetHttpHeader() {
    protocol = 1.1;
    status   = HTTP_RESP_OK;
    strcpy ( server, srvName );
    contentLen    = 0;
    acceptRanges  = 0;
    cCtrl  = 0;
    date = time ( NULL );
    strcpy ( contentType, "text/plain; charset=UTF-8" );
	termId[0] = 0;
    location[0]   = 0;
    contentEnc[0] = 0;
    connection    = CONN_CLOSE;
    lastModified  = 0;
    expires       = 0;
    data          = 0;
}

long long int Connection::indexCount = 1;

void sendRemainderData ( Connection *conn ) {
    if ( conn->resp.getContentLen() < 0 ) {
        if ( conn->len > 0 ) {
            if ( conn->len <= CHUNK_HDR_SIZE ) {
                lastChunk ( conn->buf, conn->len );
            } else {
                if ( SMALLBUF - conn->len  < 8 ) {
                    truncateChunk ( &conn->buf[2], conn->len );
                    conn->len = sendConnectionData ( conn, conn->buf, conn->len );
                    lastChunk ( conn->buf, conn->len );
                } else {
                    truncateChunk ( &conn->buf[2], conn->len );
                    lastChunk ( &conn->buf[conn->len], conn->len );
                }
            }
        } else {
            lastChunk ( conn->buf, conn->len );
        }
    } else {

        if ( conn->len > 0 ) {
            unsigned int tempLen = 0;
            tempLen = sendConnectionData ( conn, conn->buf, conn->len );

            if ( tempLen != conn->len ) {
                debuglog (  "ERRR: Unable to send complete data\n" );
            }
        }
    }

    conn->len = 0;
}

unsigned int sendConnectionData (    Connection *conn,
                                     unsigned char *buf,
                                     unsigned int len ) {
/*
    if ( conn->resp.getContentLen() < 0 ) {
        //Use chunked transfer code here
        return 0;
    }
*/
#ifdef USE_SSL

    if ( conn->ssl ) {
        unsigned int bytesW = 0;
        int temp  = 0;
        int retry = 0;
        debuglog (  "INFO: SSL_write: Sending %d bytes \n", len );

        do {
            temp = SSL_write ( conn->ssl, buf + bytesW, len - bytesW  );

            if ( temp > 0 ) {
                bytesW += temp;
                debuglog (  "DBUG: SSL_write: Sent %d bytes, sent=%d, data size=%d\n", temp, bytesW, len );
            } else {
                int rc = 0;
                retry++;

                if ( ( rc = SSL_get_error ( conn->ssl, temp ) ) == SSL_ERROR_WANT_WRITE ) {
                    debuglog (  "WARN: SSL_write : Problem sending data %d, retrying...\n", rc );

                    if ( retry >= 24 )
                    { break; }

                    std::this_thread::sleep_for ( std::chrono::microseconds ( 10 ) ); /*PR_Sleep ( 1 );*/
                } else {
                    debuglog (  "ERRR: SSL_write : Problem sending data %d\n", rc );
                    bytesW = 0;
                    break;
                }
            }
        } while ( bytesW < len );

        if ( bytesW != len ) {
            debuglog (  "ERRR: SSL_write: Unable to send all data, bytes sent=%d, total bytes=%d\n", bytesW, len );
            len = len + 1;
        } else {
            len = 0;
        }

        return len;

    } else {
#endif
        PRFileDesc *sock = conn->socket;
        unsigned int bytesW = 0;
        int temp = 0;
        PRErrorCode error = 0;
        debuglog (  "INFO: Write: Sending %d bytes \n", len );

        do {
            //temp = PR_Write ( sock, buf + bytesW, len - bytesW  );
            temp = PR_Send ( sock, buf + bytesW, len - bytesW  );

            if ( temp > 0 ) {
                bytesW += temp;
                debuglog (  "DBUG: Write: Sent %d bytes, sent=%d, data size=%d\n", temp, bytesW, len );
            } else {
                error = PR_GetError();

                if ( error == EWOULDBLOCK || error == EAGAIN ) {
                    debuglog (  "WARN: Write: Blocking, temp=%d \n", temp );
                    std::this_thread::sleep_for ( std::chrono::microseconds ( 10 ) ); /*PR_Sleep ( 1 );*/
                } else {
                    debuglog (  "ERRR: Write: Problem sending data %d,%d\n", error, errno );
                    bytesW = 0;
                    break;
                }
            }
        } while ( bytesW < len );

        if ( bytesW != len ) {
            debuglog (  "ERRR: Write: Unable to send all data, bytes sent %d, data size %d, error %d)\n", bytesW, len, error );
            len = len + 1;
        } else {
            len = 0;
        }

        return len;
#ifdef USE_SSL
    }

#endif
}

unsigned int sendConnectionDataToSock (    PRFileDesc *sock,
        unsigned char *buf,
        unsigned int len ) {
    unsigned int bytesW = 0;
    int temp = 0;
    PRErrorCode error = 0;
    debuglog (  "INFO: Sending %d bytes \n", len );

    do {
        //temp = PR_Write ( sock, buf + bytesW, len - bytesW  );
        temp = PR_Send ( sock, buf + bytesW, len - bytesW  );

        if ( temp > 0 ) {
            bytesW += temp;
            debuglog (  "DBUG: Sent %d bytes, totalsent=%d , totaldatalen=%d\n", temp, bytesW, len );
        } else {
            debuglog (  "DBUG: Less Sent %d bytes, totalsent=%d , totaldatalen=%d\n", temp, bytesW, len );
            error = PR_GetError();

            if ( error == EWOULDBLOCK || error == EAGAIN ) {
                debuglog (  "WARN: Blocking, temp=%d \n", temp );
                std::this_thread::sleep_for ( std::chrono::microseconds ( 10 ) ); /*PR_Sleep ( 1 );*/
            } else {
                debuglog (  "ERRR: Problem sending data %d,%d\n", error, errno );
                bytesW = 0;
                break;
            }
        }
    } while ( bytesW < len );

    if ( bytesW != len ) {
        debuglog (  "ERRR: Unable to send data (%d,%d,%d)\n", bytesW, len, error );
        len = len + 1;
    } else {
        len = 0;
    }

    return len;
}

unsigned int sendConnRespHdr ( Connection *conn, int status ) {
    HttpResp *tempResp = & ( conn->resp );
    debuglog ( " Request for: %s -> Status code : %d \n", conn->req.getReqFile(), status );
    string statusMsg = getStatusInfo ( status );
    tempResp->setStatus ( status );
    tempResp->setAddOn ( 1 );

    if ( status != 200 ) {
        tempResp->setContentType ( "text/plain" );
        tempResp->setContentLen ( statusMsg.length() );
    }

    conn->len  = tempResp->getHeader ( ( char * ) conn->buf );
    conn->len += conn->sess->dumpSessionCookies ( ( char * ) & ( conn->buf[conn->len] ) );
    conn->len += tempResp->finishHdr ( ( char * ) & ( conn->buf[conn->len] ) );

    if ( status != 200 ) {
        strcpy ( ( char* ) &conn->buf[conn->len], statusMsg.c_str() );
        conn->len += tempResp->getContentLen();
    }

    conn->buf[conn->len] = 0;

    char *tempbuf = ( char * ) conn->buf;
    debuglog (  "DBUG: Response Header:\n%s\n", tempbuf );
    debuglog (  "DBUG: ____________________________________\n" );
    int tempLen = sendConnectionData ( conn, conn->buf, conn->len );
    conn->len = 0;
    return tempLen;
}
