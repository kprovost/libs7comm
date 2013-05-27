#include "profinet.h"
#include "profinet_types.h"
#include "ppkt.h"
#include "pdu.h"

#include <stddef.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

struct profinet_dev* profinet_connect(const char *plc_addr)
{
    assert(plc_addr);
    int ret;
    struct profinet_dev *dev = NULL;
    struct hostent *hostent = NULL;

    hostent = gethostbyname(plc_addr);
    if (! hostent)
        return NULL;

    if (! hostent->h_addr)
        return NULL;

    dev = malloc(sizeof(struct profinet_dev));
    if (! dev)
        return NULL;

    dev->fd = socket(AF_INET, SOCK_STREAM, 0);
    if (dev->fd == -1)
        goto error;

    struct sockaddr_in in;
    in.sin_family = AF_INET;
    in.sin_port = htons(PROFINET_PORT);
    memcpy(&in.sin_addr, hostent->h_addr, sizeof(struct in_addr));

    ret = connect(dev->fd, (struct sockaddr*)&in,
            sizeof(struct sockaddr_in));
    if (ret == -1)
        goto error;

    return dev;

error:
    if (dev && dev->fd != -1)
        close(dev->fd);

    free(dev);

    return NULL;
}

void profinet_disconnect(struct profinet_dev *dev)
{
    assert(dev);
    close(dev->fd);
    free(dev);
}

static profinet_err_t profinet_create_read_request(int db, int number, struct ppkt_t **p)
{
    assert(p);

    *p = ppkt_create(sizeof(struct profinet_request));
    if (! *p)
        return PROFINET_ERR_NO_MEM;

    return PROFINET_ERR_NONE;
}

profinet_err_t profinet_read_word(struct profinet_dev *dev, int db, int number, uint16_t *value)
{
    assert(dev);
    assert(value);

    struct ppkt_t *p;
    profinet_err_t err = profinet_create_read_request(db, number, &p);
    if (! PROFINET_OK(err))
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
    if (! PROFINET_OK(err))
        goto exit;

    // TODO wait for and parse reply

exit:
    ppkt_free(p);

    return err;
}
