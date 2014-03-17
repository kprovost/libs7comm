#ifndef _TPKT_H_
#define _TPKT_H_

#include "err.h"
#include "ppkt.h"
#include "proto.h"

struct proto_t tpkt_proto;

void* tpkt_connect(const char *addr, ppkt_receive_function_t func,
        void *user, struct proto_t *lower_layer);
void tpkt_disconnect(void *dev);
err_t tpkt_receive(struct ppkt_t *p, void *user);
err_t tpkt_send(void *dev, struct ppkt_t *p);
err_t tpkt_poll(void *dev);

#endif
