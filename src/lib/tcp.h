#ifndef _TCP_H_
#define _TCP_H_

#include "err.h"
#include "ppkt.h"
#include "proto.h"

extern struct proto_t tcp_proto;

void* tcp_open(const char* addr, ppkt_receive_function_t receive,
        void *user, proto_stack_t *protos);
err_t tcp_connect(void *dev);
void tcp_disconnect(void *dev);
void tcp_close(void *dev);
err_t tcp_send(void *dev, struct ppkt_t *p);
err_t tcp_poll(void *dev);

#endif
