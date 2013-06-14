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
};

struct ppkt_t* profinet_create_request_hdr(enum profinet_function_t function, size_t payload_size)
{
    struct ppkt_t *p = ppkt_alloc(sizeof(struct profinet_hdr_t));

    struct profinet_hdr_t *hdr = (struct profinet_hdr_t*)ppkt_payload(p);
    hdr->version = PROFINET_VERSION;
    hdr->msgtype = 1;
    hdr->zero = 0;
    hdr->seq = 0;
    hdr->plen = htons(payload_size + sizeof(struct profinet_request_t));
    hdr->dlen = 0;

    struct ppkt_t *r = ppkt_alloc(sizeof(struct profinet_request_t));
    struct profinet_request_t *req = (struct profinet_request_t*)ppkt_payload(r);

    req->function = function;
    req->unknown = 1;

    return ppkt_prefix_header(p, r);
}

static err_t profinet_receive(struct ppkt_t *p, void *user)
{
    assert(p);

    ppkt_free(p);
    return ERR_NONE;
}

static err_t profinet_open_connection(struct profinet_dev_t *dev)
{
    assert(dev);
    assert(dev->cotpdev);

    struct ppkt_t *hdr = profinet_create_request_hdr(profinet_function_open_connection, sizeof(struct profinet_open_connection_t));

    struct ppkt_t *p = ppkt_alloc(sizeof(struct profinet_open_connection_t));
    struct profinet_open_connection_t *conn = (struct profinet_open_connection_t*)ppkt_payload(p);
    conn->unknown1 = htons(1);
    conn->unknown2 = htons(1);
    conn->unknown3 = htons(0x03c0);

    p = ppkt_prefix_header(hdr, p);

    err_t err = cotp_send(dev->cotpdev, p);

    // TODO wait for response?

    return err;
}

struct profinet_dev_t* profinet_connect(const char *addr)
{
    assert(addr);

    struct profinet_dev_t *dev = malloc(sizeof(struct profinet_dev_t));

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
}

err_t profinet_read_word(struct profinet_dev_t *dev, int db, int number, uint16_t *value)
{
    assert(dev);
    assert(value);

    struct ppkt_t *hdr = profinet_create_request_hdr(profinet_function_read, sizeof(struct profinet_read_request_t));

    struct ppkt_t *p = ppkt_alloc(sizeof(struct profinet_read_request_t));
    struct profinet_read_request_t *req = (struct profinet_read_request_t*)ppkt_payload(p);

    req->prefix = htons(0x120a); // TODO
    req->unknown = 0x10;
    req->read_size = 2;
    req->read_length = htons(0x0001); // TODO
    req->db_num = htons(db);
    req->area_code = profinet_area_DB;
    req->start_addr_2 = htons(number);

    p = ppkt_prefix_header(hdr, p);

    err_t err = cotp_send(dev->cotpdev, p);
    if (! OK(err))
        return err;

    // TODO wait for and parse reply

    return err;
}
