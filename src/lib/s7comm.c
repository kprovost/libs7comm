#include "s7comm.h"
#include "s7comm_types.h"
#include "ppkt.h"
#include "cotp.h"
#include "tpkt.h"
#include "tcp.h"

#include <assert.h>
#include <stdlib.h>
#include <arpa/inet.h>

struct s7comm_dev_t
{
    struct cotp_dev_t *cotpdev;
    uint16_t seq;
    struct ppkt_t *last_response;
};

static struct ppkt_t* s7comm_create_request_hdr(struct s7comm_dev_t *dev,
        enum s7comm_function_t function, size_t payload_size, size_t data_size)
{
    assert(dev);

    struct ppkt_t *p = ppkt_alloc(sizeof(struct s7comm_hdr_t));

    struct s7comm_hdr_t *hdr = PPKT_GET(struct s7comm_hdr_t, p);
    hdr->version = S7COMM_VERSION;
    hdr->msgtype = 1;
    hdr->zero = 0;
    hdr->seq = htons(dev->seq++);
    hdr->plen = htons(payload_size + sizeof(struct s7comm_request_t));
    hdr->dlen = htons(data_size);

    struct ppkt_t *r = ppkt_alloc(sizeof(struct s7comm_request_t));
    struct s7comm_request_t *req = PPKT_GET(struct s7comm_request_t, r);

    req->function = function;
    req->unknown = 1;

    return ppkt_prefix_header(p, r);
}

static struct ppkt_t* s7comm_process_read(struct ppkt_t *p, err_t *err)
{
    assert(p);

    if (ppkt_size(p) < sizeof(struct s7comm_read_response_t))
    {
        *err = ERR_READ_FAILURE;
        ppkt_free(p);
        return NULL;
    }

    struct s7comm_read_response_t *resp = PPKT_GET(struct s7comm_read_response_t, p);
    uint16_t length = ntohs(resp->len);
    if (resp->len_type == 4)
        length >>= 3;

    if (resp->err != s7comm_READ_RESPONSE_ERR_NONE)
    {
        if (resp->err == s7comm_READ_RESPONSE_ERR_NO_ITEM)
            *err = ERR_NO_SUCH_VALUE;
        else
            *err = ERR_READ_FAILURE;
        ppkt_free(p);
        return NULL;
    }
    ppkt_pull(p, sizeof(struct s7comm_read_response_t));
    assert(length == ppkt_size(p));

    *err = ERR_NONE;
    return p;
}

static struct ppkt_t* s7comm_process_write(struct ppkt_t *p, err_t *err)
{
    assert(p);

    if (ppkt_size(p) < sizeof(struct s7comm_write_response_t))
    {
        *err = ERR_WRITE_FAILURE;
        ppkt_free(p);
        return NULL;
    }

    struct s7comm_write_response_t *resp = PPKT_GET(struct s7comm_write_response_t, p);
    if (resp->err != s7comm_READ_RESPONSE_ERR_NONE)
    {
        if (resp->err == s7comm_READ_RESPONSE_ERR_NO_ITEM)
            *err = ERR_NO_SUCH_VALUE;
        else
            *err = ERR_READ_FAILURE;
        ppkt_free(p);
        return NULL;
    }

    *err = ERR_NONE;
    return p;
}

static err_t s7comm_receive(struct ppkt_t *p, void *user)
{
    assert(p);
    assert(user);
    assert(ppkt_chain_count(p) == 1);

    struct s7comm_dev_t *dev = (struct s7comm_dev_t*)user;

    assert(! dev->last_response);
    dev->last_response = p;
    return ERR_NONE;
}

static struct ppkt_t* s7comm_process_receive(struct ppkt_t *p, err_t *err)
{
    struct s7comm_hdr_t *hdr = PPKT_GET(struct s7comm_hdr_t, p);
    uint16_t plen = ntohs(hdr->plen);
    uint16_t dlen = ntohs(hdr->dlen);

    if (plen < 2)
    {
        *err = ERR_READ_FAILURE;
        // Short packet?
        goto done;
    }

    ppkt_pull(p, sizeof(struct s7comm_hdr_t));

    if (hdr->msgtype == 2 || hdr->msgtype == 3)
        // Result, if we're interested.
        ppkt_pull(p, 2);

    struct s7comm_request_t *req = PPKT_GET(struct s7comm_request_t, p);
    ppkt_pull(p, sizeof(struct s7comm_request_t));

    if (ppkt_size(p) < (plen - 2 + dlen))
    {
        *err = ERR_READ_FAILURE;
        // Invalid packet?
        goto done;
    }

    switch (req->function)
    {
        case s7comm_function_open_connection:
            // Yay, but we don't care about the content
            *err = ERR_NONE;
            break;
        case s7comm_function_read:
            return s7comm_process_read(p, err);
        case s7comm_function_write:
            return s7comm_process_write(p, err);
    }

done:
    ppkt_free(p);
    return NULL;
}

static err_t s7comm_open_connection(struct s7comm_dev_t *dev)
{
    assert(dev);
    assert(dev->cotpdev);

    struct ppkt_t *hdr = s7comm_create_request_hdr(dev,
            s7comm_function_open_connection,
            sizeof(struct s7comm_open_connection_t), 0);

    struct ppkt_t *p = ppkt_alloc(sizeof(struct s7comm_open_connection_t));
    struct s7comm_open_connection_t *conn = PPKT_GET(struct s7comm_open_connection_t, p);
    conn->unknown1 = htons(1);
    conn->unknown2 = htons(1);
    conn->unknown3 = htons(0x03c0);

    p = ppkt_prefix_header(hdr, p);

    err_t err = cotp_send(dev->cotpdev, p);
    if (! OK(err))
        return err;

    err = cotp_poll(dev->cotpdev);
    if (! OK(err))
        return err;

    if (! dev->last_response)
        return ERR_CONNECTION_CLOSED;

    // TODO: Don't just assume that connecting succeeded
    struct ppkt_t *r = s7comm_process_receive(dev->last_response, &err);
    assert(! r);
    dev->last_response = NULL;

    return err;
}

struct s7comm_dev_t* s7comm_connect(const char *addr)
{
    assert(addr);

    struct s7comm_dev_t *dev = malloc(sizeof(struct s7comm_dev_t));

    dev->seq = 0;
    dev->last_response = NULL;

    struct proto_t *protostack[] = { &tpkt_proto, &tcp_proto, NULL };
    dev->cotpdev = cotp_open(addr, s7comm_receive, dev, protostack);
    if (! dev->cotpdev)
    {
        free(dev);
        return NULL;
    }

    err_t err = cotp_connect(dev->cotpdev);
    if (! OK(err))
    {
        cotp_close(dev->cotpdev);
        free(dev);
        return NULL;
    }

    err = s7comm_open_connection(dev);
    if (! OK(err))
    {
        free(dev);
        return NULL;
    }

    return dev;
}

void s7comm_disconnect(struct s7comm_dev_t *dev)
{
    if (! dev) return;
    cotp_disconnect(dev->cotpdev);
    cotp_close(dev->cotpdev);
    free(dev);
}

static err_t s7comm_do_read_request(
        struct s7comm_dev_t *dev,
        enum s7comm_area_t area,
        int db,
        uint32_t start_addr,
        enum s7comm_read_size_t size)
{
    assert(dev);

    struct ppkt_t *hdr = s7comm_create_request_hdr(dev,
            s7comm_function_read,
            sizeof(struct s7comm_read_request_t), 0);

    struct ppkt_t *p = ppkt_alloc(sizeof(struct s7comm_read_request_t));
    struct s7comm_read_request_t *req = PPKT_GET(struct s7comm_read_request_t, p);

    req->prefix = htons(0x120a); // TODO
    req->unknown = 0x10;
    req->read_size = size;
    req->read_length = htons(1); // Number of words to read
    req->db_num = htons(db);
    req->area_code = area;
    req->start_addr = (start_addr & 0x00ff0000) >> 24;
    req->start_addr_2 = htons(start_addr & 0xffff);

    p = ppkt_prefix_header(hdr, p);

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

static int s7comm_bit_size(enum s7comm_read_size_t size)
{
    switch (size)
    {
        case s7comm_read_size_bit:
            return 1;
        case s7comm_read_size_byte:
            return 8;
        case s7comm_read_size_word:
            return 16;
    }
    assert(false);
    return 0;
}

static err_t s7comm_do_write_request(
        struct s7comm_dev_t *dev,
        enum s7comm_area_t area,
        int db,
        uint32_t start_addr,
        enum s7comm_read_size_t size,
        struct ppkt_t *value)
{
    assert(dev);

    int bit_size = s7comm_bit_size(size);
    struct ppkt_t *hdr = s7comm_create_request_hdr(dev,
            s7comm_function_write,
            sizeof(struct s7comm_read_request_t),
            sizeof(struct s7comm_read_response_t) + ppkt_size(value));

    struct ppkt_t *p = ppkt_alloc(sizeof(struct s7comm_read_request_t));
    struct s7comm_read_request_t *req = PPKT_GET(struct s7comm_read_request_t, p);

    req->prefix = htons(0x120a);
    req->unknown = 0x10;
    req->read_size = size;
    req->read_length = htons(1); // Number of words to read
    req->db_num = htons(db);
    req->area_code = area;
    req->start_addr = (start_addr & 0x00ff0000) >> 24;
    req->start_addr_2 = htons(start_addr & 0xffff);

    p = ppkt_prefix_header(hdr, p);

    struct ppkt_t *rr = ppkt_alloc(sizeof(struct s7comm_read_response_t));
    struct s7comm_read_response_t *resp = PPKT_GET(struct s7comm_read_response_t, rr);
    resp->err = s7comm_READ_RESPONSE_ERR_NONE;
    resp->len_type = ( size == s7comm_read_size_bit ? 3 : 4 );
    resp->len = htons(bit_size);

    p = ppkt_append_footer(rr, p);
    p = ppkt_append_footer(value, p);

    // Send
    err_t err = cotp_send(dev->cotpdev, p);
    if (! OK(err))
        return ERR_WRITE_FAILURE;

    // Wait for reply
    err = cotp_poll(dev->cotpdev);
    if (! OK(err))
        return ERR_WRITE_FAILURE;

    if (! dev->last_response)
        return ERR_WRITE_FAILURE;

    struct ppkt_t *r = s7comm_process_receive(dev->last_response, &err);
    dev->last_response = NULL;

    if (! r)
        return ERR_WRITE_FAILURE;

    ppkt_free(r);
    return err;
}

err_t s7comm_read_db_bit(struct s7comm_dev_t *dev, int db, int number, bool *value)
{
    assert(dev);
    assert(value);

    uint32_t start_addr = number;
    err_t err = s7comm_do_read_request(dev, s7comm_area_DB, db, start_addr, s7comm_read_size_bit);
    if (! OK(err))
        return err;

    if (! dev->last_response)
        return ERR_READ_FAILURE;

    struct ppkt_t *r = s7comm_process_receive(dev->last_response, &err);
    if (! r || ! OK(err))
    {
        dev->last_response = NULL;
        return err;
    }

    assert(ppkt_size(r) == 1);
    *value = *PPKT_GET(uint8_t, r);

    ppkt_free(r);
    dev->last_response = NULL;
    return ERR_NONE;
}

err_t s7comm_read_db_byte(struct s7comm_dev_t *dev, int db, int number, uint8_t *value)
{
    assert(dev);
    assert(value);

    uint32_t start_addr = number * 8;
    err_t err = s7comm_do_read_request(dev, s7comm_area_DB, db, start_addr, s7comm_read_size_byte);
    if (! OK(err))
        return err;

    if (! dev->last_response)
        return ERR_READ_FAILURE;

    struct ppkt_t *r = s7comm_process_receive(dev->last_response, &err);
    if (! r || ! OK(err))
    {
        dev->last_response = NULL;
        return err;
    }

    assert(ppkt_size(r) == 1);
    *value = *PPKT_GET(uint8_t, r);

    ppkt_free(r);
    dev->last_response = NULL;
    return err;
}

err_t s7comm_read_db_word(struct s7comm_dev_t *dev, int db, int number, uint16_t *value)
{
    assert(dev);
    assert(value);

    uint32_t start_addr = number * 8;
    err_t err = s7comm_do_read_request(dev, s7comm_area_DB, db, start_addr, s7comm_read_size_word);
    if (! OK(err))
        return err;

    if (! dev->last_response)
        return ERR_READ_FAILURE;

    struct ppkt_t *r = s7comm_process_receive(dev->last_response, &err);
    if (! r || ! OK(err))
    {
        dev->last_response = NULL;
        return ERR_READ_FAILURE;
    }

    assert(ppkt_size(r) == 2);
    uint16_t *res = PPKT_GET(uint16_t, r);
    *value = ntohs(*res);

    ppkt_free(r);
    dev->last_response = NULL;
    return err;
}

err_t s7comm_write_db_bit(struct s7comm_dev_t *dev, int db, int number, uint8_t value)
{
    assert(dev);

    struct ppkt_t *p = ppkt_alloc(1);
    uint8_t *write_val = PPKT_GET(uint8_t, p);
    *write_val = !! value;

    uint32_t start_addr = number;
    return s7comm_do_write_request(dev, s7comm_area_DB, db, start_addr, s7comm_read_size_bit, p);
}

err_t s7comm_write_db_byte(struct s7comm_dev_t *dev, int db, int number, uint8_t value)
{
    assert(dev);

    struct ppkt_t *p = ppkt_alloc(1);
    uint8_t *write_val = PPKT_GET(uint8_t, p);
    *write_val = value;

    uint32_t start_addr = number * 8;
    return s7comm_do_write_request(dev, s7comm_area_DB, db, start_addr, s7comm_read_size_byte, p);
}

err_t s7comm_write_db_word(struct s7comm_dev_t *dev, int db, int number, uint16_t value)
{
    assert(dev);

    struct ppkt_t *p = ppkt_alloc(2);
    uint16_t *write_val = PPKT_GET(uint16_t, p);
    *write_val = htons(value);

    uint32_t start_addr = number * 8;
    return s7comm_do_write_request(dev, s7comm_area_DB, db, start_addr, s7comm_read_size_word, p);
}

err_t s7comm_read_output(struct s7comm_dev_t *dev, int card, int port, bool *value)
{
    assert(dev);
    assert(value);

    err_t err = s7comm_do_read_request(dev, s7comm_area_Outputs, card, port, s7comm_read_size_bit);
    if (! OK(err))
        return err;

    if (! dev->last_response)
        return ERR_READ_FAILURE;

    struct ppkt_t *r = s7comm_process_receive(dev->last_response, &err);
    if (! r || ! OK(err))
    {
        dev->last_response = NULL;
        return err;
    }

    assert(ppkt_size(r) == 1);
    *value = *PPKT_GET(uint8_t, r);

    ppkt_free(r);
    dev->last_response = NULL;
    return ERR_NONE;
}

err_t s7comm_read_flag_bit(struct s7comm_dev_t *dev, int number, bool *value)
{
    assert(dev);
    assert(value);

    uint32_t start_addr = number;
    err_t err = s7comm_do_read_request(dev, s7comm_area_Flags, 0, start_addr, s7comm_read_size_bit);
    if (! OK(err))
        return err;

    if (! dev->last_response)
        return ERR_READ_FAILURE;

    struct ppkt_t *r = s7comm_process_receive(dev->last_response, &err);
    if (! r || ! OK(err))
    {
        dev->last_response = NULL;
        return err;
    }

    assert(ppkt_size(r) == 1);
    *value = *PPKT_GET(uint8_t, r);

    ppkt_free(r);
    dev->last_response = NULL;
    return ERR_NONE;
}

err_t s7comm_write_flag_bit(struct s7comm_dev_t *dev, int number, bool value)
{
    assert(dev);

    struct ppkt_t *p = ppkt_alloc(1);
    uint8_t *write_val = PPKT_GET(uint8_t, p);
    *write_val = !! value;

    uint32_t start_addr = number;
    return s7comm_do_write_request(dev, s7comm_area_Flags, 0, start_addr, s7comm_read_size_bit, p);

}
