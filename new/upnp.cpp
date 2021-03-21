/*
 * This file Copyright (C) 2007-2014 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: upnp.c 14262 2014-04-27 19:31:10Z jordan $
 */

#include <assert.h>
#include <errno.h>
#include <stdio.h>
//#include <iostream>

//#ifdef SYSTEM_MINIUPNP
//  #include <miniupnpc/miniupnpc.h>
//  #include <miniupnpc/upnpcommands.h>
//#else

//#endif

//#include "transmission.h"
//#include "log.h"
//#include "port-forwarding.h"
//#include "session.h"
#include "upnp.h"
//#include "utils.h"

//static const char *
//getKey (void) { return _("Port Forwarding (UPnP)"); }



/**
***
**/

tr_upnp*
tr_upnpInit (void)
{
    //tr_upnp * ret = new0 (tr_upnp, 1);
    tr_upnp * ret = new tr_upnp;

    ret->state = TR_UPNP_DISCOVER;
    ret->oldState = TR_UPNP_IDLE;
    ret->port = -1;
    return ret;
}

void
tr_upnpClose (tr_upnp * handle)
{
    assert (!handle->isMapped);
    assert ((handle->state == TR_UPNP_IDLE)
          || (handle->state == TR_UPNP_ERR)
          || (handle->state == TR_UPNP_DISCOVER));

    if (handle->hasDiscovered)
        FreeUPNPUrls (&handle->urls);
    //tr_free (handle);
    delete handle;
}

/**
***  Wrappers for miniupnpc functions
**/

static struct UPNPDev *
tr_upnpDiscover (int msec)
{
  struct UPNPDev * ret;
  bool have_err;

#if (MINIUPNPC_API_VERSION >= 8) /* adds ipv6 and error args */
  fprintf (stderr, "DBUG: Using MINIUPNP API VERSION = %d \n",MINIUPNPC_API_VERSION );
  int err = UPNPDISCOVER_SUCCESS;
  ret = upnpDiscover (msec, NULL, NULL, 0, 0, &err);
  have_err = err != UPNPDISCOVER_SUCCESS;
#else
  ret = upnpDiscover (msec, NULL, NULL, 0);
  have_err = ret == NULL;
#endif

  if (have_err)
    //tr_logAddNamedDbg (getKey (), "upnpDiscover failed (errno %d - %s)", errno, tr_strerror (errno));
      fprintf ( stderr,"ERRR: upnpDiscover failed (errno %d) \n", errno  );

  return ret;
}

static int
tr_upnpGetSpecificPortMappingEntry (tr_upnp * handle, const char * proto)
{
    int err;
    char intClient[16];
    char intPort[16];
    char portStr[16];

    *intClient = '\0';
    *intPort = '\0';

    //tr_snprintf (portStr, sizeof(portStr), "%d", (int)handle->port);
    snprintf (portStr, sizeof(portStr), "%d", (int)handle->port);

#if (MINIUPNPC_API_VERSION >= 10) /* adds remoteHost arg */

    err = UPNP_GetSpecificPortMappingEntry (handle->urls.controlURL, 
                                            handle->data.first.servicetype, 
                                            portStr,
                                            proto,
                                            NULL /*remoteHost*/, 
                                            intClient,
                                            intPort,
                                            NULL /*desc*/, 
                                            NULL /*enabled*/,
                                            NULL /*duration*/);

#elif (MINIUPNPC_API_VERSION >= 8) /* adds desc, enabled and leaseDuration args */

    err = UPNP_GetSpecificPortMappingEntry (handle->urls.controlURL,
                                            handle->data.first.servicetype,
                                            portStr,
                                            proto,
                                            intClient,
                                            intPort,
                                            NULL /*desc*/,
                                            NULL /*enabled*/,
                                            NULL /*duration*/);

#else

    err = UPNP_GetSpecificPortMappingEntry (handle->urls.controlURL,
                                            handle->data.first.servicetype,
                                            portStr,
                                            proto,
                                            intClient,
                                            intPort);

#endif

    return err;
}

static int
tr_upnpAddPortMapping (const tr_upnp * handle, const char * proto, int port, const char * desc)
{
    int err;
    const int old_errno = errno;
    char portStr[16];
    errno = 0;

    //tr_snprintf (portStr, sizeof (portStr), "%d", (int)port);
    snprintf (portStr, sizeof (portStr), "%d", (int)port);

#if (MINIUPNPC_API_VERSION >= 8)
    err = UPNP_AddPortMapping (handle->urls.controlURL, handle->data.first.servicetype, portStr, portStr, handle->lanaddr, desc, proto, NULL, NULL);
#else
    err = UPNP_AddPortMapping (handle->urls.controlURL, handle->data.first.servicetype, portStr, portStr, handle->lanaddr, desc, proto, NULL);
#endif

    if (err)
        //tr_logAddNamedDbg (getKey (), "%s Port forwarding failed with error %d (errno %d - %s)", proto, err, errno, tr_strerror (errno));
        fprintf ( stderr, "ERRR: %s Port forwarding failed with error %d (errno %d)\n", proto, err, errno);

    errno = old_errno;
    return err;
}

static void
tr_upnpDeletePortMapping (const tr_upnp * handle, const char * proto, int port)
{
    char portStr[16];

    //tr_snprintf (portStr, sizeof (portStr), "%d", (int)port);
    snprintf (portStr, sizeof (portStr), "%d", (int)port);

    UPNP_DeletePortMapping (handle->urls.controlURL,
                            handle->data.first.servicetype,
                            portStr, proto, NULL);
}

/**
***
**/

enum
{
  UPNP_IGD_NONE = 0,
  UPNP_IGD_VALID_CONNECTED = 1,
  UPNP_IGD_VALID_NOT_CONNECTED = 2,
  UPNP_IGD_INVALID = 3
};

int
tr_upnpPulse (tr_upnp * handle,
              int       port,
              int       isEnabled,
              int       doPortCheck)
{
    int ret;

    fprintf ( stderr, "INFO: tr_upnpPulse(): on port %d \n", port );
    //tr_upnp_state oldState = TR_UPNP_IDLE;
    //if ( handle->state != TR_UPNP_IDLE )
        handle->oldState = handle->state;

    if (isEnabled && (handle->state == TR_UPNP_DISCOVER))
    {
        struct UPNPDev * devlist;

        devlist = tr_upnpDiscover (2000);

        errno = 0;
        if (UPNP_GetValidIGD (devlist, &handle->urls, &handle->data,
                             handle->lanaddr, sizeof (handle->lanaddr)) == UPNP_IGD_VALID_CONNECTED)
        {
            fprintf ( stderr, "INFO: Found Internet Gateway Device \"%s\"\n", handle->urls.controlURL);
            fprintf ( stderr, "INFO: Local Address is \"%s\"\n", handle->lanaddr);
            //tr_logAddNamedInfo (getKey (), _(
            //             "Found Internet Gateway Device \"%s\""),
            //         handle->urls.controlURL);
            //tr_logAddNamedInfo (getKey (), _(
            //             "Local Address is \"%s\""), handle->lanaddr);
            handle->state = TR_UPNP_IDLE;
            handle->hasDiscovered = 1;
        }
        else
        {
            handle->state = TR_UPNP_ERR;
            fprintf ( stderr, "ERRR: UPNP_GetValidIGD failed (errno %d)\n", errno);
            //tr_logAddNamedDbg (
            //     getKey (), "UPNP_GetValidIGD failed (errno %d - %s)",
            //    errno,
            //    tr_strerror (errno));
            //tr_logAddNamedDbg (
            //    getKey (),
            //    "If your router supports UPnP, please make sure UPnP is enabled!");
            fprintf ( stderr,"INFO: If your router supports UPnP, please make sure UPnP is enabled!\n" );
        }
        freeUPNPDevlist (devlist);
    }

    if (handle->state == TR_UPNP_IDLE)
    {
        if (handle->isMapped && (!isEnabled || (handle->port != port)))
            handle->state = TR_UPNP_UNMAP;
    }

    if (isEnabled && handle->isMapped && doPortCheck)
    {
        if ((tr_upnpGetSpecificPortMappingEntry (handle, "TCP") != UPNPCOMMAND_SUCCESS) ||
          (tr_upnpGetSpecificPortMappingEntry (handle, "UDP") != UPNPCOMMAND_SUCCESS))
        {
            //tr_logAddNamedInfo (getKey (), _("Port %d isn't forwarded"), handle->port);
            fprintf (stderr, "INFO: Port %d isn't forwarded\n", handle->port);
            handle->isMapped = false;
        }
    }

    if (handle->state == TR_UPNP_UNMAP)
    {
        tr_upnpDeletePortMapping (handle, "TCP", handle->port);
        tr_upnpDeletePortMapping (handle, "UDP", handle->port);

        //tr_logAddNamedInfo (getKey (),
        //         _("Stopping port forwarding through \"%s\", service \"%s\""),
        //         handle->urls.controlURL, handle->data.first.servicetype);
        fprintf ( stderr, "INFO: Stopping port forwarding through \"%s\" service \"%s\"\n",
                 handle->urls.controlURL, handle->data.first.servicetype);

        handle->isMapped = 0;
        handle->state = TR_UPNP_IDLE;
        handle->port = -1;
    }

    if (handle->state == TR_UPNP_IDLE)
    {
        if (isEnabled && !handle->isMapped)
            handle->state = TR_UPNP_MAP;
    }

    if (handle->state == TR_UPNP_MAP)
    {
        int  err_tcp = -1;
        int  err_udp = -1;
        errno = 0;

        if (!handle->urls.controlURL || !handle->data.first.servicetype)
            handle->isMapped = 0;
        else
        {
            char desc[64];
            snprintf (desc, sizeof (desc), "%s at %d", "AppServer", port);

            err_tcp = tr_upnpAddPortMapping (handle, "TCP", port, desc);
            err_udp = tr_upnpAddPortMapping (handle, "UDP", port, desc);

            handle->isMapped = !err_tcp | !err_udp;
        }
        //tr_logAddNamedInfo (getKey (),
        //         _("Port forwarding through \"%s\", service \"%s\". (local address: %s:%d)"),
        fprintf ( stderr, "INFO: Port forwarding through \"%s\", service \"%s\". (local address: %s:%d)\n",
                 handle->urls.controlURL, handle->data.first.servicetype,
                 handle->lanaddr, port);
        if (handle->isMapped)
        {
            //tr_logAddNamedInfo (getKey (), "%s", _("Port forwarding successful!"));
            fprintf(stderr, "INFO: Port forwarding successful!\n");
            handle->port = port;
            handle->state = TR_UPNP_IDLE;
        }
        else
        {
            //tr_logAddNamedDbg (getKey (), "If your router supports UPnP, please make sure UPnP is enabled!");
            fprintf(stderr, "If your router supports UPnP, please make sure UPnP is enabled!\n");
            handle->port = -1;
            handle->state = TR_UPNP_ERR;
        }
    }

    if ( handle->oldState != handle->state )
        fprintf ( stderr, "DBUG: UPNP Changed state from %d to %d \n", handle->oldState, handle->state);
    else
        fprintf ( stderr, "DBUG: UPNP No state change =  %d \n", handle->state);
    switch (handle->state)
    {
        case TR_UPNP_DISCOVER:
            ret = TR_PORT_UNMAPPED; break;

        case TR_UPNP_MAP:
            ret = TR_PORT_MAPPING; break;

        case TR_UPNP_UNMAP:
            ret = TR_PORT_UNMAPPING; break;

        case TR_UPNP_IDLE:
            ret = handle->isMapped ? TR_PORT_MAPPED
                  : TR_PORT_UNMAPPED; break;

        default:
            ret = TR_PORT_ERROR; break;
    }

    return ret;
}

