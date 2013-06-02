#ifndef _TPKT_H_
#define _TPKT_H_

#include "err.h"
#include "ppkt.h"

struct tpkt_dev_t;

struct tpkt_dev_t* tpkt_connect(const char *addr, ppkt_receive_function_t func);
err_t tpkt_send(struct tpkt_dev_t *dev, struct ppkt_t *p);

#endif
