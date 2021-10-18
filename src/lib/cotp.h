#ifndef _COTP_H_
#define _COTP_H_

#include "err.h"
#include "ppkt.h"
#include "proto.h"

extern struct proto_t cotp_proto;

void* cotp_open(const char *addr, ppkt_receive_function_t receive,
        void *user, proto_stack_t *protos);
err_t cotp_connect(void* dev);
void cotp_disconnect(void *dev);
void cotp_close(void *dev);
err_t cotp_send(void *dev, struct ppkt_t *p);
err_t cotp_poll(void *dev);

#endif
