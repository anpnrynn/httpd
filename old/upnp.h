/*
 * This file Copyright (C) 2007-2014 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: upnp.h 14241 2014-01-21 03:10:30Z jordan $
 */


//#ifndef __TRANSMISSION__
//#error only libtransmission should #include this header.
//#endif

#ifndef TR_UPNP_H
#define TR_UPNP_H 1

//#include <iostream>
//using namespace std;
#include "portforwarding.h"
#ifndef LINUX_BUILD
#ifdef  USE_MINIUPNP
#include <miniupnp/miniupnpc.h>
#include <miniupnp/upnpcommands.h>
#endif
#else
#include <miniupnpc/miniupnpc.h>
#include <miniupnpc/upnpcommands.h>
#endif


/**
 * @addtogroup port_forwarding Port Forwarding
 * @{
 */

typedef enum
{
    TR_UPNP_IDLE,
    TR_UPNP_ERR,
    TR_UPNP_DISCOVER,
    TR_UPNP_MAP,
    TR_UPNP_UNMAP
}
tr_upnp_state;

struct tr_upnp
{
    bool               hasDiscovered;
    struct UPNPUrls    urls;
    struct IGDdatas    data;
    int                port;
    char               lanaddr[16];
    unsigned int       isMapped;
    tr_upnp_state      state;
    tr_upnp_state      oldState;
};

typedef struct tr_upnp tr_upnp;

tr_upnp * tr_upnpInit (void);

void      tr_upnpClose (tr_upnp *);

int       tr_upnpPulse (      tr_upnp *,
                            int port,
                            int isEnabled,
                            int doPortCheck);
/* @} */
#endif
