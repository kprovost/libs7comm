#ifndef _COTP_H_
#define _COTP_H_

#include "err.h"
#include "ppkt.h"

struct cotp_dev_t;

struct cotp_dev_t* cotp_connect(const char *addr, ppkt_receive_function_t receive, void *user);
void cotp_disconnect(struct cotp_dev_t *dev);
err_t cotp_send(struct cotp_dev_t *dev, struct ppkt_t *p);
err_t cotp_poll(struct cotp_dev_t *dev);

#endif
