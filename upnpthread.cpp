#include "upnpthread.h"

UpnpThread::UpnpThread ( int portNumber ) {
    port = portNumber;
}

void UpnpThread::run() {
    std::cerr << "DBUG: Upnp thread started " << std::endl;
    tr_upnp *upnp = tr_upnpInit();

    while ( 1 ) {
        int err = tr_upnpPulse ( upnp, port, true, true );

        if ( err != TR_PORT_MAPPED ) {
            QThread::sleep ( 1 );

            if ( err == TR_PORT_ERROR ) {
                std::cerr << "ERRR: Upnp port forwarding error... exiting " << std::endl;
                break;
            }

            std::cerr << "DBUG: Upnp thread short sleep" << std::endl;
            continue;
        } else {
            QThread::sleep ( 10 );
        }

        std::cerr << "DBUG: Upnp thread sleeping " << std::endl;
    }

    tr_upnpClose ( upnp );
}
