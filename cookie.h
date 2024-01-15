#ifndef COOKIE_MGR_H
#define COOKIE_MGR_H

#include <apptypes.h>

class CookieMgr {

    private:

    protected:
        COOKIES *cookies;
    public:
        CookieMgr();
        ~CookieMgr();

        bool getCookie   ( string, string& );
        void delCookie   ( string );
        bool checkCookie ( string, string );
        void setCookie   ( string, string );
        void readCookies ( unsigned char *, int, int );
        void clearCookies();
};

#endif
