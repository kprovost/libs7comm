#include "ppkt.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <assert.h>

#define PPKT_FLAG_ALLOCATED 1

struct ppkt_t
{
    uint8_t       *payload;
    size_t         size;
    size_t         offset;
    struct ppkt_t *next;
    uint8_t        flags;
};

struct ppkt_t *ppkt_alloc(size_t size)
{
    struct ppkt_t *p = malloc(sizeof(struct ppkt_t));
    memset(p, 0, sizeof(struct ppkt_t));

    p->payload = malloc(size);
    p->size = size;
    p->flags = PPKT_FLAG_ALLOCATED;

    return p;
}

struct ppkt_t *ppkt_create(uint8_t *data, size_t size)
{
    struct ppkt_t *p = malloc(sizeof(struct ppkt_t));
    memset(p, 0, sizeof(struct ppkt_t));

    p->payload = data;
    p->size = size;
    p->flags = 0;

    return p;
}

void ppkt_free(struct ppkt_t *p)
{
    if (! p)
        return;

    assert(p->payload);

    if (p->next)
        ppkt_free(p->next);

    if (! (p->flags & PPKT_FLAG_ALLOCATED))
            free(p->payload);
    free(p);
}

struct ppkt_t *ppkt_prefix_header(struct ppkt_t *hdr, struct ppkt_t *p)
{
    hdr->next = p;
    return hdr;
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

err_t ppkt_send(int fd, struct ppkt_t *p)
{
    assert(fd != -1);
    assert(p);

    size_t size = ppkt_chain_size(p);

    size_t pkts = ppkt_chain_count(p);
    assert(pkts >= 1);
    struct iovec *iov = malloc(sizeof(struct iovec) * pkts);
    if (! iov)
        return ERR_NO_MEM;

    for (size_t i = 0; i < pkts; i++)
    {
        iov[i].iov_base = ppkt_payload(p);
        iov[i].iov_len = ppkt_size(p);

        p = p->next;
    }

    ssize_t ret = writev(fd, iov, pkts);
    assert(size == (size_t)ret);

    free(iov);

    return ret == -1 ? ERR_SEND_FAILED : ERR_NONE;
}
