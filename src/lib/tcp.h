#ifndef _TCP_H_
#define _TCP_H_

#include "ppkt.h"

struct tcp_dev_t;

struct tcp_dev_t* tcp_connect(const char* addr, uint16_t port, ppkt_receive_function_t receive, void *user);
void tcp_disconnect(struct tcp_dev_t *dev);
err_t tcp_send(struct tcp_dev_t *dev, struct ppkt_t *p);
err_t tcp_poll(struct tcp_dev_t *dev);

#endif
