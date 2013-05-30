#include "ppkt.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <assert.h>

struct ppkt_t
{
    uint8_t       *payload;
    size_t         size;
    size_t         offset;
    struct ppkt_t *next;
};

struct ppkt_t *ppkt_create(size_t size)
{
    struct ppkt_t *p = malloc(sizeof(struct ppkt_t));
    memset(p, 0, sizeof(struct ppkt_t));

    p->payload = malloc(size);
    p->size = size;

    return p;
}

void ppkt_free(struct ppkt_t *p)
{
    if (! p)
        return;

    assert(p->payload);

    if (p->next)
        ppkt_free(p->next);

    free(p->payload);
    free(p);
}

struct ppkt_t *ppkt_prefix_header(struct ppkt_t *hdr, struct ppkt_t *p)
{
    p->next = hdr;
    return p;
}

uint8_t* ppkt_payload(struct ppkt_t *p)
{
    return p->payload + p->offset;
}

size_t ppkt_size(struct ppkt_t *p)
{
    return p->size;
}

size_t ppkt_chain_size(struct ppkt_t *p)
{
    assert(p);
    size_t length = 0;

    while (p)
    {
        length += p->size;
        p = p->next;
    }

    return length;
}

size_t ppkt_chain_count(struct ppkt_t *p)
{
    assert(p);
    size_t length = 0;

    while (p)
    {
        length++;
        p = p->next;
    }

    return length;
}

profinet_err_t ppkt_send(int fd, struct ppkt_t *p)
{
    assert(fd != -1);
    assert(p);

    size_t size = ppkt_chain_size(p);

    size_t pkts = ppkt_chain_count(p);
    assert(pkts >= 1);
    struct iovec *iov = malloc(sizeof(struct iovec) * pkts);
    if (! iov)
        return PROFINET_ERR_NO_MEM;

    for (size_t i = 0; i < pkts; i++)
    {
        iov[i].iov_base = ppkt_payload(p);
        iov[i].iov_len = ppkt_size(p);

        p = p->next;
    }

    ssize_t ret = writev(fd, iov, pkts);
    assert(size == (size_t)ret);

    free(iov);

    return ret == -1 ? PROFINET_ERR_SEND_FAILED : PROFINET_ERR_NONE;
}
