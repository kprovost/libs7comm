#include "tpkt.h"
#include "tcp.h"
#include "pcap.h"

#include <assert.h>
#include <stdlib.h>

#define TPKT_PORT 102

struct tpkt_dev_t
{
    ppkt_receive_function_t receive;
    struct tcp_dev_t  *tcp;
    struct pcap_dev_t *pcap;
};

static err_t tpkt_receive(struct ppkt_t *p)
{
    assert(p);

    return ERR_UNKNOWN;
}

struct tpkt_dev_t* tpkt_connect(const char *addr, ppkt_receive_function_t receive)
{
    assert(addr);
    assert(receive);

    struct tpkt_dev_t *dev = malloc(sizeof(struct tpkt_dev_t));
    if (! dev)
        return NULL;

    dev->tcp = NULL;
    dev->pcap = NULL;
    dev->receive = receive;

    /* Little special here, but it's easy to test.
     *
     * We try to 'connect' to a pcap file, if that fails we try to use the addr
     * as a TCP target instead */
    dev->pcap = pcap_connect(addr, tpkt_receive);
    if (! dev->pcap)
    {
        dev->tcp = tcp_connect(addr, TPKT_PORT, tpkt_receive);

        if (! dev->tcp)
        {
            free(dev);
            dev = NULL;
        }
    }

    return dev;
}

err_t tpkt_t_send(struct tpkt_dev_t *dev, struct ppkt_t *p)
{
    return ERR_UNKNOWN;
}
