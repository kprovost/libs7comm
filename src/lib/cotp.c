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
};

enum cotp_tpdu_code_t
{
    COTP_TPDU_CONNECT_REQUEST = 0xE0,
    COTP_TPDU_CONNECT_CONFIRM = 0xD0,
    COTP_TPDU_DATA            = 0xF0,
};

struct cotphdr_common_t
{
    uint8_t size;
    uint8_t tpdu_code;
} __attribute__((packed));

struct cotphdr_connect_t
{
    struct cotphdr_common_t common;
    uint16_t dst_ref;
    uint16_t src_ref;
    uint8_t flags;
} __attribute__((packed));

struct cotphdr_data_t
{
    struct cotphdr_common_t common;
    uint8_t flags;
} __attribute__((packed));

static err_t cotp_receive_connect_confirm(struct cotp_dev_t *dev, struct ppkt_t *p)
{
    return ERR_NONE;
}

static err_t cotp_receive_data(struct cotp_dev_t *dev, struct ppkt_t *p)
{
    return ERR_NONE;
}

static err_t cotp_receive(struct ppkt_t *p, void *user)
{
    assert(p);
    assert(user);

    struct cotp_dev_t *dev = (struct cotp_dev_t*)user;

    /* Note: We get complete packets from the tpkt layer, so it *should* be
     * impossoble for us to get incomplete packets.
     * There's still a pktqueue on the cotp layer, because cotp too can
     * fragment (data) packets.
     */
    size_t pktsize = ppkt_chain_size(p);
    assert(pktsize > sizeof(struct cotphdr_common_t));

    struct cotphdr_common_t *cotphdr = (struct cotphdr_common_t*)ppkt_payload(p);
    uint8_t cotpsize = cotphdr->size;

    assert(pktsize >= cotpsize);

    if (cotphdr->tpdu_code == COTP_TPDU_CONNECT_CONFIRM)
        return cotp_receive_connect_confirm(dev, p);
    else if (cotphdr->tpdu_code == COTP_TPDU_DATA)
        return cotp_receive_data(dev, p);
    else
    {
        ppkt_free(p);
        return ERR_NOT_SUPPORTED;
    }
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
    dev->tpkt = tpkt_connect(addr, cotp_receive, dev);
    if (! dev->tpkt)
    {
        free(dev);
        return NULL;
    }

    // TODO Send out connect message (and wait for reply??)

    return dev;
}

void cotp_disconnect(struct cotp_dev_t *dev)
{
    assert(dev);

    // TODO Send disconnect message? Wait for confirmation?

    tpkt_disconnect(dev->tpkt);

    free(dev);
}

err_t cotp_poll(struct cotp_dev_t *dev)
{
    assert(dev);

    return tpkt_poll(dev->tpkt);
}
