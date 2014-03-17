#ifndef _TCP_H_
#define _TCP_H_

#include "err.h"
#include "ppkt.h"
#include "proto.h"

struct proto_t tcp_proto;

void* tcp_connect(const char* addr, ppkt_receive_function_t receive,
        void *user, struct proto_t *lower_layer);
void tcp_disconnect(void *dev);
err_t tcp_send(void *dev, struct ppkt_t *p);
err_t tcp_poll(void *dev);

#endif
