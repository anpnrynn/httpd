#include <cookie.h>

CookieMgr::CookieMgr() {
    cookies = new COOKIES;
}

CookieMgr::~CookieMgr() {
    cookies->clear();
    delete cookies;
}

void CookieMgr::clearCookies() {
    cookies->clear();
}

void CookieMgr::delCookie ( string cName ) {
    COOKIES::iterator i = cookies->find ( cName );

    if ( i != cookies->end() ) {
        cookies->erase ( i );
    }
}

void CookieMgr::readCookies ( unsigned char *buf, int len, int maxLen ) {
    int i = 0;

    if ( len <= 2 || len > maxLen )
    { return; }

    int j = 0;
    char  name[256];
    char value[256];
    char *data = name;

    while ( ( i < ( len - 2 ) ) && ( buf[i] != '\r' && buf[i + 1] != '\n' ) ) {
        switch ( buf[i] ) {
            case ';':
                data[j] = 0;
                cookies->insert ( pair<string, string> ( name, value ) );
                data = name;
                j = 0;
                break;

            case '=':
                data[j] = 0;
                data = value;
                j = 0;
                break;

            case ' ':
                if ( j > 0 )
                { data[j++] = ' '; }

                break;

            default :
                data[j++] = buf[i];
                break;
        }

        i++;
    }
}

void CookieMgr::setCookie ( string name, string value ) {
    cookies->insert ( pair<string, string> ( name, value ) );
}

bool CookieMgr::checkCookie ( string name, string value ) {
    COOKIES::iterator i = cookies->find ( name );

    if ( i != cookies->end() ) {
        if ( i->second == value )
        { return true; }
    }

    return false;
}

bool CookieMgr::getCookie ( string name, string &value ) {
    COOKIES::iterator i = cookies->find ( name );

    if ( i != cookies->end() ) {
        value = i->second;
        return true;
    }

    return false;
}
