#include "profinet.h"
#include "profinet_types.h"
#include "ppkt.h"
#include "cotp.h"

#include <assert.h>
#include <stdlib.h>
#include <arpa/inet.h>

struct profinet_dev_t
{
    struct cotp_dev_t *cotpdev;
    uint16_t seq;
    struct ppkt_t *last_response;
};

struct ppkt_t* profinet_create_request_hdr(struct profinet_dev_t *dev,
        enum profinet_function_t function, size_t payload_size, size_t data_size)
{
    assert(dev);

    struct ppkt_t *p = ppkt_alloc(sizeof(struct profinet_hdr_t));

    struct profinet_hdr_t *hdr = (struct profinet_hdr_t*)ppkt_payload(p);
    hdr->version = PROFINET_VERSION;
    hdr->msgtype = 1;
    hdr->zero = 0;
    hdr->seq = htons(dev->seq++);
    hdr->plen = htons(payload_size + sizeof(struct profinet_request_t));
    hdr->dlen = htons(data_size);

    struct ppkt_t *r = ppkt_alloc(sizeof(struct profinet_request_t));
    struct profinet_request_t *req = (struct profinet_request_t*)ppkt_payload(r);

    req->function = function;
    req->unknown = 1;

    return ppkt_prefix_header(p, r);
}

static err_t profinet_receive_read(struct profinet_dev_t *dev, struct ppkt_t *p)
{
    assert(dev);
    assert(p);

    struct profinet_read_response_t *resp = (struct profinet_read_response_t*)ppkt_payload(p);
    uint16_t length = ntohs(resp->len);
    if (resp->len_type == 4)
        length >>= 3;

    if (resp->err != 0xff)
    {
        ppkt_free(p);
        return ERR_READ_FAILURE;
    }
    ppkt_pull(p, sizeof(struct profinet_read_response_t));
    assert(length == ppkt_size(p));

    assert(! dev->last_response);
    dev->last_response = p;

    return ERR_NONE;
}

static err_t profinet_receive(struct ppkt_t *p, void *user)
{
    assert(p);
    assert(user);
    assert(ppkt_chain_count(p) == 1);

    struct profinet_dev_t *dev = (struct profinet_dev_t*)user;

    struct profinet_hdr_t *hdr = (struct profinet_hdr_t*)ppkt_payload(p);
    uint16_t plen = ntohs(hdr->plen);
    uint16_t dlen = ntohs(hdr->dlen);

    if (plen < 2)
        // Short packet?
        goto done;

    ppkt_pull(p, sizeof(struct profinet_hdr_t));

    if (hdr->msgtype == 2 || hdr->msgtype == 3)
    {
        // Result, if we're interested.
        ppkt_pull(p, 2);
    }

    struct profinet_request_t *req = (struct profinet_request_t*)ppkt_payload(p);
    ppkt_pull(p, sizeof(struct profinet_request_t));

    if (ppkt_size(p) < (plen - 2 + dlen))
        // Invalid packet?
        goto done;

    switch (req->function)
    {
        case profinet_function_open_connection:
            // Yay, but we don't care about the content
            break;
        case profinet_function_read:
            return profinet_receive_read(dev, p);
    }

done:
    ppkt_free(p);
    return ERR_NONE;
}

static err_t profinet_open_connection(struct profinet_dev_t *dev)
{
    assert(dev);
    assert(dev->cotpdev);

    struct ppkt_t *hdr = profinet_create_request_hdr(dev,
            profinet_function_open_connection,
            sizeof(struct profinet_open_connection_t), 0);

    struct ppkt_t *p = ppkt_alloc(sizeof(struct profinet_open_connection_t));
    struct profinet_open_connection_t *conn = (struct profinet_open_connection_t*)ppkt_payload(p);
    conn->unknown1 = htons(1);
    conn->unknown2 = htons(1);
    conn->unknown3 = htons(0x03c0);

    p = ppkt_prefix_header(hdr, p);

    err_t err = cotp_send(dev->cotpdev, p);
    if (! OK(err))
        return err;

    // TODO: Don't just assume we got the expected response!
    return cotp_poll(dev->cotpdev);
}

struct profinet_dev_t* profinet_connect(const char *addr)
{
    assert(addr);

    struct profinet_dev_t *dev = malloc(sizeof(struct profinet_dev_t));

    dev->seq = 0;
    dev->last_response = NULL;
    dev->cotpdev = cotp_connect(addr, profinet_receive, dev);
    if (! dev->cotpdev)
    {
        free(dev);
        return NULL;
    }

    err_t err = profinet_open_connection(dev);
    if (! OK(err))
    {
        free(dev);
        return NULL;
    }

    return dev;
}

void profinet_disconnect(struct profinet_dev_t *dev)
{
    assert(dev);
    cotp_disconnect(dev->cotpdev);
    free(dev);
}

static err_t profinet_do_read_request(
        struct profinet_dev_t *dev,
        int db,
        uint32_t start_addr,
        enum profinet_read_size_t size)
{
    assert(dev);

    struct ppkt_t *hdr = profinet_create_request_hdr(dev,
            profinet_function_read,
            sizeof(struct profinet_read_request_t), 0);

    struct ppkt_t *p = ppkt_alloc(sizeof(struct profinet_read_request_t));
    struct profinet_read_request_t *req = (struct profinet_read_request_t*)ppkt_payload(p);

    req->prefix = htons(0x120a); // TODO
    req->unknown = 0x10;
    req->read_size = size;
    req->read_length = htons(1); // Number of words to read
    req->db_num = htons(db);
    req->area_code = profinet_area_DB;
    req->start_addr = (start_addr & 0x00ff0000) >> 24;
    req->start_addr_2 = htons(start_addr & 0xffff);

    p = ppkt_prefix_header(hdr, p);
    if (! p)
        return ERR_NO_MEM;

    err_t err = cotp_send(dev->cotpdev, p);
    if (! OK(err))
        return err;

    err = cotp_poll(dev->cotpdev);
    if (! OK(err))
        return err;

    if (! dev->last_response)
        return ERR_READ_FAILURE;

    return ERR_NONE;
}

err_t profinet_read_bit(struct profinet_dev_t *dev, int db, int number, bool *value)
{
    assert(dev);
    assert(value);

    uint32_t start_addr = number;
    err_t err = profinet_do_read_request(dev, db, start_addr, profinet_read_size_bit);
    if (! OK(err))
        return err;

    assert(ppkt_size(dev->last_response) == 1);

    *value = *ppkt_payload(dev->last_response);

    ppkt_free(dev->last_response);
    dev->last_response = NULL;

    return ERR_NONE;
}

err_t profinet_read_byte(struct profinet_dev_t *dev, int db, int number, uint8_t *value)
{
    assert(dev);
    assert(value);

    uint32_t start_addr = number * 8;
    err_t err = profinet_do_read_request(dev, db, start_addr, profinet_read_size_byte);
    if (! OK(err))
        return err;

    assert(ppkt_size(dev->last_response) == 1);

    *value = *ppkt_payload(dev->last_response);

    ppkt_free(dev->last_response);
    dev->last_response = NULL;

    return ERR_NONE;
}

err_t profinet_read_word(struct profinet_dev_t *dev, int db, int number, uint16_t *value)
{
    assert(dev);
    assert(value);

    uint32_t start_addr = number * 8;
    err_t err = profinet_do_read_request(dev, db, start_addr, profinet_read_size_word);
    if (! OK(err))
        return err;

    assert(ppkt_size(dev->last_response) == 2);

    uint16_t *res = (uint16_t*)ppkt_payload(dev->last_response);
    *value = ntohs(*res);

    ppkt_free(dev->last_response);
    dev->last_response = NULL;

    return ERR_NONE;
}
