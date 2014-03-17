#ifndef _COTP_H_
#define _COTP_H_

#include "err.h"
#include "ppkt.h"
#include "proto.h"

struct proto_t cotp_proto;

void* cotp_connect(const char *addr, ppkt_receive_function_t receive,
        void *user, struct proto_t *lower_layer);
void cotp_disconnect(void *dev);
err_t cotp_send(void *dev, struct ppkt_t *p);
err_t cotp_poll(void *dev);

#endif
