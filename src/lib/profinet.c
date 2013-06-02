#include "profinet.h"
#include "profinet_types.h"
#include "ppkt.h"
#include "pdu.h"

#include <assert.h>
#include <arpa/inet.h>

struct profinet_dev* profinet_connect(const char *plc_addr)
{
    return NULL;
}

void profinet_disconnect(struct profinet_dev *dev)
{
    assert(dev);
}

static err_t profinet_create_read_request(int db, int number, struct ppkt_t **p)
{
    assert(p);

    *p = ppkt_alloc(sizeof(struct profinet_request));
    if (! *p)
        return ERR_NO_MEM;

    return ERR_NONE;
}

err_t profinet_read_word(struct profinet_dev *dev, int db, int number, uint16_t *value)
{
    assert(dev);
    assert(value);

    struct ppkt_t *p;
    err_t err = profinet_create_read_request(db, number, &p);
    if (! OK(err))
        return err;

    struct profinet_request *req = (struct profinet_request*)ppkt_payload(p);
    req->function = profinet_function_read;
    req->prefix = htons(0x120a); // TODO
    req->read_size = 2;
    req->bytes = htons(0x1000); // TODO
    req->db_num = htons(db);
    req->area_code = profinet_area_DB;
    req->start_addr = htons(number);

    err = profinet_pdu_send(dev, &p);
    if (! OK(err))
        goto exit;

    // TODO wait for and parse reply

exit:
    ppkt_free(p);

    return err;
}
