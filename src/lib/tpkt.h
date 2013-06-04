#ifndef _TPKT_H_
#define _TPKT_H_

#include "err.h"
#include "ppkt.h"

struct tpkt_dev_t;

struct tpkt_dev_t* tpkt_connect(const char *addr, ppkt_receive_function_t func, void *user);
void tpkt_disconnect(struct tpkt_dev_t *dev);
err_t tpkt_send(struct tpkt_dev_t *dev, struct ppkt_t *p);
err_t tpkt_poll(struct tpkt_dev_t *dev);

#endif
