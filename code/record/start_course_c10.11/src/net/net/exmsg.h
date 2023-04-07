#ifndef EXMSG_H
#define EXMSG_H

#include "net_errr.h"
#include "nlist.h"
#include "netif.h"

typedef struct _exmsg_t {
    nlist_node_t node;
    
    enum {
        NET_EXMSG_NETIF_IN,
    }type;

    int id;
}exmsg_t;

net_err_t exmsg_init (void);
net_err_t exmsg_start (void);
net_err_t exmsg_netif_in(netif_t * netif);

#endif 
