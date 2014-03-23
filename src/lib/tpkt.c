#include "tpkt.h"

#include <assert.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "tcp.h"

struct tpkt_dev_t
{
    ppkt_receive_function_t receive;
    void *lower_dev;
    struct proto_t *proto;
    void *user;
    struct ppkt_t *pktqueue;
};

struct tpkthdr_t
{
    uint8_t version;
    uint8_t reserved;
    uint16_t size;
} __attribute__((packed));

err_t tpkt_receive(struct ppkt_t *p, void *user)
{
    assert(p);

    struct tpkt_dev_t *dev = (struct tpkt_dev_t*)user;
    assert(dev);

    dev->pktqueue = ppkt_append_footer(p, dev->pktqueue);

    while (ppkt_chain_size(dev->pktqueue) >= sizeof(struct tpkthdr_t))
    {
        dev->pktqueue = ppkt_coalesce(dev->pktqueue, sizeof(struct tpkthdr_t));

        struct tpkthdr_t *tpkthdr = PPKT_GET(struct tpkthdr_t, dev->pktqueue);

        assert(tpkthdr->version == 3);

        size_t tpkt_size = ntohs(tpkthdr->size);
        if (ppkt_chain_size(dev->pktqueue) < tpkt_size)
            // Need more data
            return ERR_NONE;

        // Split the chain into before and after
        struct ppkt_t *pkt = dev->pktqueue;
        ppkt_split(pkt, &dev->pktqueue, tpkt_size);

        // Skip header in the arrived packet
        ppkt_pull(pkt, sizeof(struct tpkthdr_t));

        // Pass the result to our upper layer
        assert(dev->receive);
        err_t ret = dev->receive(pkt, dev->user);
        if (ret != ERR_NONE)
            return ret;
    }

    return ERR_NONE;
}

void* tpkt_open(const char *addr, ppkt_receive_function_t receive,
        void *user, proto_stack_t *protos)
{
    assert(addr);
    assert(receive);
    assert(protos);
    assert(*protos);

    struct tpkt_dev_t *dev = malloc(sizeof(struct tpkt_dev_t));

    dev->proto = *protos;

    dev->lower_dev = NULL;
    dev->receive = receive;
    dev->user = user;
    dev->pktqueue = NULL;

    dev->lower_dev = dev->proto->proto_open(addr, tpkt_receive, dev,
            protos + 1);

    if (! dev->lower_dev)
    {
        free(dev);
        dev = NULL;
    }

    return dev;
}

err_t tpkt_connect(void *d)
{
    assert(d);
    struct tpkt_dev_t *dev = (struct tpkt_dev_t*)d;

    return dev->proto->proto_connect(dev->lower_dev);
}

void tpkt_disconnect(void *d)
{
    assert(d);
    struct tpkt_dev_t *dev = (struct tpkt_dev_t*)d;

    if (dev->lower_dev)
        dev->proto->proto_disconnect(dev->lower_dev);
}

void tpkt_close(void *d)
{
    assert(d);
    struct tpkt_dev_t *dev = (struct tpkt_dev_t*)d;

    dev->proto->proto_close(dev->lower_dev);

    ppkt_free(dev->pktqueue);
    free(dev);
}

err_t tpkt_send(void *d, struct ppkt_t *p)
{
    assert(d);
    assert(p);

    struct tpkt_dev_t *dev = (struct tpkt_dev_t*)d;

    struct ppkt_t *hdr = ppkt_alloc(sizeof(struct tpkthdr_t));
    struct tpkthdr_t *tpkthdr = PPKT_GET(struct tpkthdr_t, hdr);

    tpkthdr->version = 3;
    tpkthdr->reserved = 0;
    tpkthdr->size = htons(ppkt_chain_size(p) + sizeof(struct tpkthdr_t));

    p = ppkt_prefix_header(hdr, p);

    assert(dev->lower_dev);
    return dev->proto->proto_send(dev->lower_dev, p);
}

err_t tpkt_poll(void *d)
{
    assert(d);

    struct tpkt_dev_t *dev = (struct tpkt_dev_t*)d;

    return dev->proto->proto_poll(dev->lower_dev);
}

struct proto_t tpkt_proto = {
    .name = "TPKT",
    .proto_open = tpkt_open,
    .proto_connect = tpkt_connect,
    .proto_disconnect = tpkt_disconnect,
    .proto_close = tpkt_close,
    .proto_receive = tpkt_receive,
    .proto_send = tpkt_send,
    .proto_poll = tpkt_poll
};
