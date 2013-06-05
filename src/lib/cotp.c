#include "cotp.h"
#include "tpkt.h"

#include <assert.h>
#include <stdlib.h>
#include <arpa/inet.h>

struct cotp_dev_t
{
    ppkt_receive_function_t receive;
    struct tpkt_dev_t *tpkt;
    void *user;

    struct ppkt_t *pktqueue;
};

struct cotphdr_t
{
    uint8_t size;
    // ???
};

static err_t cotp_receive(struct ppkt_t *p, void *user)
{
    assert(p);
    assert(user);

    return ERR_UNKNOWN;
}

struct cotp_dev_t* cotp_connect(const char *addr, ppkt_receive_function_t receive, void *user)
{
    assert(addr);
    assert(receive);

    struct cotp_dev_t *dev = malloc(sizeof(struct cotp_dev_t));
    if (! dev)
        return NULL;

    dev->receive = receive;
    dev->user = user;
    dev->pktqueue = NULL;
    dev->tpkt = tpkt_connect(addr, cotp_receive, dev);
    if (! dev->tpkt)
    {
        free(dev);
        return NULL;
    }

    return dev;
}

void cotp_disconnect(struct cotp_dev_t *dev)
{
    assert(dev);

    tpkt_disconnect(dev->tpkt);
    ppkt_free(dev->pktqueue);

    free(dev);
}

err_t cotp_poll(struct cotp_dev_t *dev)
{
    assert(dev);

    return tpkt_poll(dev->tpkt);
}
