#ifndef PORTFORWARDING_H
#define PORTFORWARDING_H

typedef enum
{
    TR_PORT_ERROR,
    TR_PORT_UNMAPPED,
    TR_PORT_UNMAPPING,
    TR_PORT_MAPPING,
    TR_PORT_MAPPED
}
tr_port_forwarding;

#endif // PORTFORWARDING_H

