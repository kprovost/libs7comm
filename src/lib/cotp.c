#include "cotp.h"
#include "tpkt.h"

#include <assert.h>
#include <stdlib.h>
#include <arpa/inet.h>

enum cotp_state_t
{
    COTP_STATE_DISCONNECTED,
    COTP_STATE_CONNECTING,
    COTP_STATE_CONNECTED,
};

struct cotp_dev_t
{
    ppkt_receive_function_t receive;
    void *lower_dev;
    struct proto_t *proto;
    void *user;
    enum cotp_state_t state;
};

enum cotp_tpdu_code_t
{
    COTP_TPDU_CONNECT_REQUEST = 0xE0,
    COTP_TPDU_CONNECT_CONFIRM = 0xD0,
    COTP_TPDU_DATA            = 0xF0,
};

enum cotp_param_t
{
    COTP_PARAM_SRC_TSAP  = 0xC1,
    COTP_PARAM_DST_TSAP  = 0xC2,
    COTP_PARAM_TPDU_SIZE = 0xC0,
};

#define COTP_NUMBER_FINAL_FRAME 0x80

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
    uint8_t number;
} __attribute__((packed));

static err_t cotp_receive_connect_confirm(struct cotp_dev_t *dev, struct ppkt_t *p)
{
    assert(dev);
    assert(p);

    size_t size = ppkt_chain_size(p);
    assert(size >= sizeof(struct cotphdr_connect_t));

    p = ppkt_coalesce(p, size);

    struct cotphdr_connect_t *conn = PPKT_GET(struct cotphdr_connect_t, p);
    assert(conn->flags == 0);

    assert(dev->state == COTP_STATE_CONNECTING);
    dev->state = COTP_STATE_CONNECTED;

    ppkt_free(p);

    return ERR_NONE;
}

static err_t cotp_receive_data(struct cotp_dev_t *dev, struct ppkt_t *p)
{
    assert(dev);
    assert(p);
    assert(dev->receive);

    size_t size = ppkt_chain_size(p);

    assert(size >= sizeof(struct cotphdr_data_t));

    p = ppkt_coalesce(p, sizeof(struct cotphdr_data_t));
    struct cotphdr_data_t *data = PPKT_GET(struct cotphdr_data_t, p);

    assert(data->common.size == sizeof(struct cotphdr_data_t) - 1);

    // We don't support cotp fragmentation (yet)
    assert(data->number & COTP_NUMBER_FINAL_FRAME);

    ppkt_pull(p, sizeof(struct cotphdr_data_t));

    return dev->receive(p, dev->user);
}

err_t cotp_receive(struct ppkt_t *p, void *user)
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

    struct cotphdr_common_t *cotphdr = PPKT_GET(struct cotphdr_common_t, p);
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

static void cotp_append_option(struct ppkt_t *p, enum cotp_param_t type, size_t size, uint32_t value)
{
    assert(p);

    struct ppkt_t *option = ppkt_alloc(2 + size);

    uint8_t *bytes = PPKT_GET(uint8_t, option);

    *bytes++ = type;
    *bytes++ = size;

    switch (size)
    {
        case 4:
            *bytes++ = value >> 24;
            *bytes++ = (value >> 16) & 0xff;
        case 2:
            *bytes++ = (value >> 8) & 0xff;
        case 1:
            *bytes++ = value & 0xff;
            break;
        default:
            assert(0);
    }

    ppkt_append_footer(option, p);
}

void* cotp_open(const char *addr, ppkt_receive_function_t receive,
        void *user, proto_stack_t *protos)
{
    assert(addr);
    assert(receive);
    assert(protos);
    assert(*protos);

    struct cotp_dev_t *dev = malloc(sizeof(struct cotp_dev_t));
    if (! dev)
        return NULL;

    dev->receive = receive;
    dev->user = user;
    dev->proto = *protos;
    dev->lower_dev = dev->proto->proto_open(addr, cotp_receive, dev, protos + 1);
    dev->state = COTP_STATE_DISCONNECTED;
    if (! dev->lower_dev)
    {
        free(dev);
        return NULL;
    }

    return dev;
}

err_t cotp_connect(void* d)
{
    assert(d);
    struct cotp_dev_t *dev = (struct cotp_dev_t*)d;

    // Send out connect message
    struct ppkt_t *p = ppkt_alloc(sizeof(struct cotphdr_connect_t));

    dev->state = COTP_STATE_CONNECTING;

    struct cotphdr_connect_t *conn = PPKT_GET(struct cotphdr_connect_t, p);
    conn->common.tpdu_code = COTP_TPDU_CONNECT_REQUEST;
    conn->dst_ref = htons(0);
    conn->src_ref = htons(1);
    conn->flags = 0;

    cotp_append_option(p, COTP_PARAM_SRC_TSAP, 2, 0x100);
    cotp_append_option(p, COTP_PARAM_DST_TSAP, 2, 0x102);
    cotp_append_option(p, COTP_PARAM_TPDU_SIZE, 1, 9); // 512

    conn->common.size = ppkt_chain_size(p) - 1;

    err_t err = dev->proto->proto_send(dev->lower_dev, p);
    if (! OK(err))
    {
        cotp_disconnect(dev);
        return err;
    }

    while (dev->state != COTP_STATE_CONNECTED)
    {
        err_t err = dev->proto->proto_poll(dev->lower_dev);
        if (! OK(err))
        {
            cotp_disconnect(dev);
            return err;
        }
    }

    return ERR_NONE;
}

void cotp_disconnect(void *d)
{
    assert(d);

    struct cotp_dev_t *dev = (struct cotp_dev_t*)d;

    // TODO Send disconnect message? Wait for confirmation?

    dev->proto->proto_disconnect(dev->lower_dev);

    dev->state = COTP_STATE_DISCONNECTED;
}

void cotp_close(void *d)
{
    assert(d);
    struct cotp_dev_t *dev = (struct cotp_dev_t*)d;

    assert(dev->state == COTP_STATE_DISCONNECTED);

    dev->proto->proto_close(dev->lower_dev);
    free(dev);
}

err_t cotp_send(void *d, struct ppkt_t *p)
{
    assert(d);
    struct cotp_dev_t *dev = (struct cotp_dev_t*)d;
    assert(dev->lower_dev);
    assert(dev->state == COTP_STATE_CONNECTED);
    assert(p);

    struct ppkt_t *hdr = ppkt_alloc(sizeof(struct cotphdr_data_t));
    struct cotphdr_data_t *datahdr = PPKT_GET(struct cotphdr_data_t, hdr);

    datahdr->common.size = sizeof(struct cotphdr_data_t) - 1;
    datahdr->common.tpdu_code = COTP_TPDU_DATA;
    datahdr->number = COTP_NUMBER_FINAL_FRAME;

    p = ppkt_prefix_header(hdr, p);

    return dev->proto->proto_send(dev->lower_dev, p);
}

err_t cotp_poll(void *d)
{
    assert(d);
    struct cotp_dev_t *dev = (struct cotp_dev_t*)d;

    return dev->proto->proto_poll(dev->lower_dev);
}

struct proto_t cotp_proto = {
    .name = "COTP",
    .proto_open = cotp_open,
    .proto_connect = cotp_connect,
    .proto_disconnect = cotp_disconnect,
    .proto_close = cotp_close,
    .proto_send = cotp_send,
    .proto_receive = cotp_receive,
    .proto_poll = cotp_poll
};
