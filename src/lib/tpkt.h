#ifndef _TPKT_H_
#define _TPKT_H_

#include "err.h"
#include "ppkt.h"
#include "proto.h"

struct proto_t tpkt_proto;

void* tpkt_open(const char *addr, ppkt_receive_function_t func,
        void *user, proto_stack_t *protos);
err_t tpkt_connect(void *dev);
void tpkt_disconnect(void *dev);
void tpkt_close(void *dev);
err_t tpkt_receive(struct ppkt_t *p, void *user);
err_t tpkt_send(void *dev, struct ppkt_t *p);
err_t tpkt_poll(void *dev);

#endif
